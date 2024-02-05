#pragma once
#include "../Serialization.h"

class TagManager
{
    friend class TagAttribute;
public:
    struct StrOrder
    {
        bool operator()(const char* left, const char* right) const
        {
            return std::strcmp(left, right) < 0;
        }
    };
    typedef set<const char*, StrOrder> TagSet;
    typedef unordered_multimap<Name, Serialization*> TaggedSerializationMap;
    typedef unordered_multimap<Name, Serialization*>::const_iterator TaggedSerializationMapIterator;
    class Iter
    {
        friend class TagManager;
        TaggedSerializationMapIterator lower;
        TaggedSerializationMapIterator upper;
        TaggedSerializationMapIterator end;
        TaggedSerializationMapIterator current;
        Serialization* baseSerialization;
        Iter();
    public:
        bool next();
        Serialization& get() const;
        void reset();
    };
    
    static TagManager& get();

    int fuzzyFindTag(const string& key, vector<Name>& tags);
    Iter findByTag(const Name& tag, Serialization* baseSerialization = NULL);
protected:
    TagSet tagSet;
    TaggedSerializationMap taggedSerialization;

    void registerTag(const Name& name, Serialization& serialization);
};

class TagAttribute : public Attribute
{
public:
    TagAttribute(const string& tag);

    bool checkTag(const Name& tag) const;

    virtual void resolve(Attribute* sourceAttribute, Serialization& serialization);
    virtual void finalize(Serialization& serialization);
protected:
    string tagTemp;
    Name tagFinal;
};
