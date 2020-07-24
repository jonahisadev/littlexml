#include "lxml.h"

int main()
{
    XMLDocument doc;
    if (XMLDocument_load(&doc, "test.xml")) {
        printf("%s: %s\n", doc.root->tag, doc.root->inner_text);
        XMLDocument_free(&doc);
    }

    return 0;
}