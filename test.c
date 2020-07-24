#include "lxml.h"

int main()
{
    XMLDocument doc;
    if (XMLDocument_load(&doc, "test.xml")) {
        printf("XML Document (version=%s, encoding=%s)\n", doc.version, doc.encoding);

        XMLNode* main_node = XMLNode_child(doc.root, 0);
        printf("Car (%s)\n", main_node->attributes.data[0].value);

        XMLDocument_free(&doc);
    }

    return 0;
}