#include "lxml.h"

int main()
{
    XMLDocument doc;
    if (XMLDocument_load(&doc, "test.xml")) {
        XMLNode* str = XMLNode_child(doc.root, 0);
        printf("Struct: %s\n", XMLNode_attr_val(str, "name"));

        for (int i = 0; i < str->children.size; i++) {
            XMLNode* field = XMLNode_child(str, i);
            printf("  %s (%s)\n", XMLNode_attr_val(field, "name"), XMLNode_attr_val(field, "type"));
        }

        XMLDocument_free(&doc);
    }

    return 0;
}