#include "TagAttribute.h"

TagManager::Iter::Iter()
    : baseSerialization(NULL)
{
}

bool TagManager::Iter::next()
{
    if (current == end)
        current = lower;
    else
        ++current;
    while (current != end
        && current != upper
        && (baseSerialization && !current->second->isChildOf(*baseSerialization))
        ) {
        ++current;
    }
    if (current == upper)
        current = end;
    return current != end;
}

Serialization& TagManager::Iter::get() const
{
    assert(current != end && current->second);
    return *current->second;
}

void TagManager::Iter::reset()
{
    current = end;
}

TagManager& TagManager::get()
{
    static TagManager instance;
    return instance;
}

int TagManager::fuzzyFindTag(const string& key, vector<Name>& tags)
{
    const int oldCount = tags.size();
    auto b = tagSet.lower_bound(key.c_str());
    string skey = key;
    skey.back() += 1;
    const auto e = tagSet.lower_bound(skey.c_str());
    const auto end = tagSet.end();
    while (b != end) {
        if (b == e)
            break;
        tags.push_back(*b);
        ++b;
    }
    return tags.size() - oldCount;
}

TagManager::Iter TagManager::findByTag(const Name& tag, Serialization* baseSerialization)
{
    const auto rawIter = taggedSerialization.equal_range(tag);
    Iter iter;
    iter.lower = rawIter.first;
    iter.upper = rawIter.second;
    iter.end = taggedSerialization.end();
    iter.baseSerialization = baseSerialization;
    iter.reset();
    return iter;
}

void TagManager::registerTag(const Name& name, Serialization& serialization)
{
    tagSet.emplace(name.c_str());
    taggedSerialization.emplace(name, &serialization);
}

TagAttribute::TagAttribute(const string& tag)
    : Attribute("Tag", true)
    , tagTemp(tag)
{
}

bool TagAttribute::checkTag(const Name& tag) const
{
    return tag == tagFinal;
}

void TagAttribute::resolve(Attribute* sourceAttribute, Serialization& serialization)
{
    if (const TagAttribute* srcTagAttr = sourceAttribute->cast<TagAttribute>()) {
        tagTemp += "." + srcTagAttr->tagTemp;
    }
}

void TagAttribute::finalize(Serialization& serialization)
{
    tagFinal = tagTemp;
    TagManager::get().registerTag(tagFinal, serialization);
}
