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

Object* findFirst(const Object& root, const Serialization& serialization)
{
    ObjectConstIterator iter(&root);
    while (iter.next()) {
        Object* obj = &iter.current();
        const Serialization& objSerialization = obj->getSerialization();
        if (&objSerialization == &serialization || objSerialization.isChildOf(serialization))
            return obj;
    }
    return NULL;
}

World* getRootWorld(const Object& object)
{
    return castTo<World>(object.getRoot());
}
