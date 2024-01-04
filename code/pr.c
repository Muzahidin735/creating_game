#include <stdio.h>
#pragma pack(1)
void main(void)
{
    struct book
    {
        char name;
        float price;
        int pages;
    };
    struct book b = {0};
    b.pages = 10;
    b.name = 127;
}