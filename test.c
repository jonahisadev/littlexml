#include "lxml.h"

int main()
{
    XMLDocument doc;
    if (XMLDocument_load(&doc, "test.xml")) {
        XMLNode* str = XMLNode_child(doc.root, 0);
        printf("Struct: %s\n", XMLNode_attr_val(str, "name"));

        XMLNodeList* fields = XMLNode_children(str, "field");
        for (int i = 0; i < fields->size; i++) {
            XMLNode* field = XMLNodeList_at(fields, i);
            printf("  %s (%s)\n", XMLNode_attr_val(field, "name"), XMLNode_attr_val(field, "type"));
        }

        XMLDocument_free(&doc);
    }

    return 0;
}