#include "Base.h"
#include "Brane.h"
#include "Console.h"

SerializeInstance(Base);

Base::Base()
{
    instanceID = Brane::registerPtr(this);
}

Base::~Base()
{
    for (auto b = linkedInstances.begin(),
        e = linkedInstances.end(); b != e; b++) {
        Base* base = (Base*)Brane::getPtrByInsID(*b);
        if (base != NULL)
            delete base;
    }
    linkedInstances.clear();
    Brane::removeInsID(instanceID);
    _magic_start_ = '\0\0\0\0';
    _magic_end_ = '\0\0\0\0';
}

bool Base::CheckValid(Base* base)
{
    if (base == NULL)
        return false;
    int* _magic_start_ = (int*)((char*)base + offsetof(Base, _magic_start_));
    int* _magic_end_ = (int*)((char*)base + offsetof(Base, _magic_end_));
    return *_magic_start_ == '$BAZ' && *_magic_end_ == 'ZAB$';
}

InstanceID Base::getInstanceID() const
{
    return instanceID;
}

bool Base::isAsset() const
{
    return !assetHandle.guid.isDefault() && assetHandle.fileID > 0;
}

bool Base::setInstanceParent(Base& base)
{
    if (&base == NULL ||
        base.getInstanceID() == 0 ||
        instanceID == 0)
        false;
    if (parentInstanceID == base.getInstanceID())
        return true;
    parentInstanceID = base.getInstanceID();
    base.linkedInstances.push_back(instanceID);
    return true;
}

InstanceAssetFile Base::getAssetFile() const
{
    return InstanceAssetFile();
}

InstanceAssetHandle Base::getAssetHandle() const
{
    return assetHandle;
}

const list<InstanceID>& Base::getLinkedInstances() const
{
    return linkedInstances;
}

Serializable* Base::instantiate(const SerializationInfo& from)
{
    return new Base();
}

bool Base::deserialize(const SerializationInfo& from)
{
    InstanceID insID;
    string guidStr;
    string path = from.path;
    Guid guid;
    if (from.get("guid", guidStr)) {
        guid = Guid::fromString(guidStr);
        assetHandle.guid = guid;
    }
    if (!guid.isDefault()) {
        if (path.empty()) {
            if (!Brane::registerGuid(instanceID, guid)) {
                Console::error("deserialize_registerGuid: Regist insID(%lld) guid(%s) failed",
                    instanceID, guid.toString().c_str());
            }
        }
        else {
            InstanceAssetFile assetFile;
            assetFile.fileID = 0;
            assetFile.highestLocalID = 0;
            assetFile.path = path;
            if (Brane::registerAssetFile(assetFile)) {
                assetHandle.fileID = assetFile.fileID;
                Brane::registerAssetHandle(instanceID, assetHandle);
            }
            else {
                Console::error("deserialize_registerAssetFile: Regist asset file(%s) at \"%s\" failed",
                    guid.toString().c_str(), assetFile.path.c_str());
            }
        }
    }
    else {
        assetHandle.fileID = 0;
    }
    return true;
}

bool Base::serialize(SerializationInfo& to)
{
    serializeInit(this, to);
    string path = to.path;
    if (!to.path.empty()) {
        if (!createAssetFromPath(path, false)) {
            Console::error("serialize_createAssetFromPath: Can not save insID(%d) to \"%s\"", instanceID, path.c_str());
            return false;
        }
    }

    to.set("guid", assetHandle.guid.toString());

    return true;
}

Base::Base(InstanceID insID, const InstanceAssetHandle& handle)
    : instanceID(insID), assetHandle(handle)
{
}

Guid Base::getOrNewGuid()
{
    if (assetHandle.guid.isDefault())
        assetHandle.guid = Guid::newGuid();
    return assetHandle.guid;
}

bool Base::createAssetFromPath(const string& path, bool allowMove)
{
    if (instanceID == 0)
        return false;

    if (isAsset()) {
        bool resave = Brane::getAssetFileByFileID(assetHandle.fileID).path == path;
        if (resave) {
            return true;
        }
        else {
            if (allowMove) {
                Brane::removeAsset(instanceID);
            }
            else
                return false;
        }
    }

    FileID fileID = Brane::getFileIDByPath(path);
    if (fileID == 0) {
        InstanceAssetFile assetFile;
        assetFile.fileID = 0;
        assetFile.highestLocalID = 0;
        assetFile.path = path;
        if (!Brane::registerAssetFile(assetFile)) {
            Console::error("createAssetFromPath_registerAssetFile: Save insID(%lld) to asset file at \"%s\" failed",
                instanceID, path.c_str());
            return false;
        }
        fileID = assetFile.fileID;
    }

    assetHandle.fileID = fileID;
    if (!Brane::addAsset(instanceID, assetHandle)) {
        Console::error("createAssetFromPath_addAsset: Save insID(%lld) to asset file at \"%s\"(FileID: %d) failed",
            instanceID, path.c_str(), fileID);
        return false;
    }
    return true;
}

bool Base::createAssetFromFileID(FileID fileID, bool allowMove)
{
    if (instanceID == 0)
        return 0;

    if (isAsset()) {
        if (allowMove) {
            Brane::removeAsset(instanceID);
        }
        else
            return false;
    }

    assetHandle.fileID = fileID;
    if (!Brane::addAsset(instanceID, assetHandle)) {
        Console::error("createAssetFromFileID_addAsset: Save insID(%lld) to asset file at (FileID: %d) failed",
            instanceID, fileID);
        return false;
    }
    return true;
}
