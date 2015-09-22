// properties.h

struct Key {
    LPCWSTR name;
    PROPERTYKEY key;
};

extern Key known[];
extern size_t known_count;
