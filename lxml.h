#ifndef LITTLE_XML_H
#define LITTLE_XML_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

//
//  Definitions
//

struct _XMLAttribute
{
    char* key;
    char* value;
};
typedef struct _XMLAttribute XMLAttribute;

void XMLAttribute_free(XMLAttribute* attr);

struct _XMLAttributeList
{
    int heap_size;
    int size;
    XMLAttribute* data;
};
typedef struct _XMLAttributeList XMLAttributeList;

void XMLAttributeList_init(XMLAttributeList* list);
void XMLAttributeList_add(XMLAttributeList* list, XMLAttribute* attr);

struct _XMLNodeList
{
    int heap_size;
    int size;
    struct _XMLNode** data;
};
typedef struct _XMLNodeList XMLNodeList;

void XMLNodeList_init(XMLNodeList* list);
void XMLNodeList_add(XMLNodeList* list, struct _XMLNode* node);

struct _XMLNode
{
    char* tag;
    char* inner_text;
    struct _XMLNode* parent;
    XMLAttributeList attributes;
    XMLNodeList children;
};
typedef struct _XMLNode XMLNode;

XMLNode* XMLNode_new(XMLNode* parent);
void XMLNode_free(XMLNode* node);
XMLNode* XMLNode_child(XMLNode* parent, int index);

struct _XMLDocument
{
    XMLNode* root;
};
typedef struct _XMLDocument XMLDocument;

int XMLDocument_load(XMLDocument* doc, const char* path);
void XMLDocument_free(XMLDocument* doc);

//
//  Implementation
//

void XMLAttribute_free(XMLAttribute* attr)
{
    free(attr->key);
    free(attr->value);
}

void XMLAttributeList_init(XMLAttributeList* list)
{
    list->heap_size = 1;
    list->size = 0;
    list->data = (XMLAttribute*) malloc(sizeof(XMLAttribute) * list->heap_size);
}

void XMLAttributeList_add(XMLAttributeList* list, XMLAttribute* attr)
{
    while (list->size >= list->heap_size) {
        list->heap_size *= 2;
        list->data = (XMLAttribute*) realloc(list->data, sizeof(XMLAttribute) * list->heap_size);
    }

    list->data[list->size++] = *attr;
}

void XMLNodeList_init(XMLNodeList* list)
{
    list->heap_size = 1;
    list->size = 0;
    list->data = (XMLNode**) malloc(sizeof(XMLNode*) * list->heap_size);
}

void XMLNodeList_add(XMLNodeList* list, XMLNode* node)
{
    while (list->size >= list->heap_size) {
        list->heap_size *= 2;
        list->data = (XMLNode**) realloc(list->data, sizeof(XMLNode*) * list->heap_size);
    }

    list->data[list->size++] = node;
}

XMLNode* XMLNode_new(XMLNode* parent)
{
    XMLNode* node = (XMLNode*) malloc(sizeof(XMLNode));
    node->parent = parent;
    node->tag = NULL;
    node->inner_text = NULL;
    XMLAttributeList_init(&node->attributes);
    XMLNodeList_init(&node->children);
    if (parent)
        XMLNodeList_add(&parent->children, node);
    return node;
}

void XMLNode_free(XMLNode* node)
{
    if (node->tag)
        free(node->tag);
    if (node->inner_text)
        free(node->inner_text);
    for (int i = 0; i < node->attributes.size; i++)
        XMLAttribute_free(&node->attributes.data[i]);
    free(node);
}

XMLNode* XMLNode_child(XMLNode* parent, int index)
{
    return parent->children.data[index];
}

int XMLDocument_load(XMLDocument* doc, const char* path)
{
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Could not load file from '%s'\n", path);
        return FALSE;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buf = (char*) malloc(sizeof(char) * size + 1);
    fread(buf, 1, size, file);
    fclose(file);
    buf[size] = '\0';

    doc->root = XMLNode_new(NULL);

    char lex[256];
    int lexi = 0;
    int i = 0;

    XMLNode* curr_node = doc->root;

    while (buf[i] != '\0')
    {
        if (buf[i] == '<') {
            lex[lexi] = '\0';

            // Inner text
            if (lexi > 0) {
                if (!curr_node) {
                    fprintf(stderr, "Text outside of document\n");
                    return FALSE;
                }

                curr_node->inner_text = strdup(lex);
                lexi = 0;
            }

            // End of node
            if (buf[i + 1] == '/') {
                i += 2;
                while (buf[i] != '>')
                    lex[lexi++] = buf[i++];
                lex[lexi] = '\0';

                if (!curr_node) {
                    fprintf(stderr, "Already at the root\n");
                    return FALSE;
                }

                if (strcmp(curr_node->tag, lex)) {
                    fprintf(stderr, "Mismatched tags (%s != %s)", curr_node->tag, lex);
                    return FALSE;
                }

                curr_node = curr_node->parent;
                i++;
                continue;
            }

            // Set current node
            curr_node = XMLNode_new(curr_node);

            // Start tag
            i++;
            XMLAttribute curr_attr = {0, 0};
            while (buf[i] != '>') {
                lex[lexi++] = buf[i++];

                // Tag name
                if (buf[i] == ' ' && !curr_node->tag) {
                    lex[lexi] = '\0';
                    curr_node->tag = strdup(lex);
                    lexi = 0;
                    i++;
                    continue;
                }

                // Usually ignore spaces
                if (lex[lexi-1] == ' ') {
                    lexi--;
                    continue;
                }

                // Attribute key
                if (buf[i] == '=') {
                    lex[lexi] = '\0';
                    curr_attr.key = strdup(lex);
                    lexi = 0;
                    continue;
                }

                // Attribute value
                if (buf[i] == '"') {
                    if (!curr_attr.key) {
                        fprintf(stderr, "Value has no key\n");
                        return FALSE;
                    }

                    lexi = 0;
                    i++;

                    while (buf[i] != '"')
                        lex[lexi++] = buf[i++];
                    lex[lexi] = '\0';
                    curr_attr.value = strdup(lex);
                    XMLAttributeList_add(&curr_node->attributes, &curr_attr);
                    curr_attr.key = NULL;
                    curr_attr.value = NULL;
                    lexi = 0;
                    i++;
                    continue;
                }
            }

            // Set tag name if none
            lex[lexi] = '\0';
            if (!curr_node->tag)
                curr_node->tag = strdup(lex);

            // Reset lexer
            lexi = 0;
            i++;
            continue;
        } else {
            lex[lexi++] = buf[i++];
        }
    }

    return TRUE;
}

void XMLDocument_free(XMLDocument* doc)
{
    XMLNode_free(doc->root);
}

#endif // LITTLE_XML_H