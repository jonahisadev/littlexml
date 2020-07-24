#include "lxml.h"

int main()
{
    XMLDocument doc;
    if (XMLDocument_load(&doc, "test.xml")) {
        XMLNode* main_node = XMLNode_child(doc.root, 0);
        printf("%d children\n", main_node->children.size);

        XMLNode* another_node = XMLNode_child(doc.root, 1);
        printf("%s\n", another_node->inner_text);

        XMLDocument_free(&doc);
    }

    return 0;
}