#define MENU_LIST_SIZE 10

typedef struct
{
    int menu_index;
    char *menu_name;
    int name_width;
    void *(*callback)(const int argv);
} menu_item_t;



