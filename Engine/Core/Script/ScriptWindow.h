#pragma once

#include "../GUI/UIWindow.h"
#include "ScriptEditor.h"

class TempScript : public ScriptBase
{
public:
    Serialize(TempScript, ScriptBase);

    TempScript(const string& name);
    
    virtual bool isValid() const;
    virtual bool load(const std::string& file);
    virtual const std::string& getName() const;
    virtual const std::string& getCodePath() const;
    virtual const std::string& getSourceCode() const;
    virtual bool setSourceCode(const std::string& code);
    virtual bool refresh();
    virtual bool saveSourceCode();

    static Serializable* instantiate(const SerializationInfo& from);
protected:
    string name;
    string code;
};

class ScriptWindow : public UIWindow
{
public:
    ScriptWindow(const string& name = "ScriptWindow", bool defaultShow = false);
    virtual ~ScriptWindow() = default;

    static void OpenScript(ScriptBase& script);
    static TempScript& OpenTempScript(const string& name, string& text);
    static TempScript& OpenTempScript(const string& path);

    virtual void AddScript(ScriptBase& script);
    virtual void AddOrSelectScript(ScriptBase& script);

    virtual void onWindowGUI(GUIRenderInfo& info);
protected:
    ScriptBase* activeScript = NULL;
    vector<Ref<ScriptBase>> scripts;
};