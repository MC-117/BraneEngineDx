#include "ObjectUltility.h"

bool isSameBranch(const Object& child, const Object& other)
{
    const Object* obj = &child;
    while (obj != NULL) {
        if (obj->parent == &other)
            return true;
        obj = obj->parent;
    }
    return false;
}
