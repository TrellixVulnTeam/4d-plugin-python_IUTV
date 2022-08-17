/* --------------------------------------------------------------------------------
 #
 #  4DPlugin-python.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : python
 #	author : miyako
 #	2021/11/17
 #  
 # --------------------------------------------------------------------------------*/

#include "4DPlugin-python.h"

#pragma mark -

#if VERSIONMAC
static bool getPythonHome(std::wstring& path) {

    NSBundle *thisBundle = [NSBundle bundleWithIdentifier:THIS_BUNDLE_ID];
    if(thisBundle){
        NSString *str = [[[thisBundle resourcePath]stringByAppendingPathComponent:@"python"]stringByAppendingPathComponent:@"lib"];
        if(str){
            NSData *u32 = [str dataUsingEncoding:NSUTF32LittleEndianStringEncoding];//default is BE
            path = std::wstring((wchar_t *)[u32 bytes], wcslen((wchar_t *)[u32 bytes]));
            return true;
         }
        
    }
    
    return false;
}
#else
static bool getPythonHome(std::wstring& path) {
    
    wchar_t    fDrive[_MAX_DRIVE],
    fDir[_MAX_DIR],
    fName[_MAX_FNAME],
    fExt[_MAX_EXT];
    
    wchar_t thisPath[_MAX_PATH] = {0};
    wchar_t resourcesPath[_MAX_PATH] = {0};
    wchar_t python_path[_MAX_PATH] = {0};
	wchar_t lib_path[_MAX_PATH] = { 0 };
    
    HMODULE hplugin = GetModuleHandleW(THIS_BUNDLE_NAME);
    GetModuleFileNameW(hplugin, thisPath, _MAX_PATH);
    
    _wsplitpath_s(thisPath, fDrive, fDir, fName, fExt);
    
    std::wstring windowsPath = fDrive;
    windowsPath += fDir;
    if(windowsPath.at(windowsPath.size() - 1) == L'\\')
        windowsPath = windowsPath.substr(0, windowsPath.size() - 1);
    
    _wsplitpath_s(windowsPath.c_str(), fDrive, fDir, fName, fExt);
    _wmakepath_s(resourcesPath, fDrive, fDir, L"Resources\\", NULL);
    
    _wsplitpath_s(resourcesPath, fDrive, fDir, fName, fExt);
    _wmakepath_s(python_path, fDrive, fDir, L"python\\", NULL);

	_wsplitpath_s(python_path, fDrive, fDir, fName, fExt);
	_wmakepath_s(lib_path, fDrive, fDir, L"lib", NULL);

    path = std::wstring(lib_path);
    
    return true;
}
#endif

static Json::Value fourd_collection_to_json(PA_CollectionRef obj, Json::Value *jsonValue) {
    
    bool parse = false;
        
    if(obj) {
        PA_Variable params[1], result;
        params[0] = PA_CreateVariable(eVK_Collection);
        PA_SetObjectVariable(&params[0], obj);
        result = PA_ExecuteCommandByID(1217, params, 1);
        PA_Unistring json = PA_GetStringVariable(result);
        CUTF16String u16;
        u16 = PA_GetUnistring(&json);
        std::string u8;
        u16_to_u8(u16, u8);
        
        Json::CharReaderBuilder builder;
        Json::CharReader *reader = builder.newCharReader();
        
        std::string errors;
        parse = reader->parse((const char *)u8.c_str(),
                              (const char *)u8.c_str() + u8.size(),
                              jsonValue,
                              &errors);
        delete reader;
    }
        
    return parse;
}

static Json::Value fourd_object_to_json(PA_ObjectRef obj, Json::Value *jsonValue) {
    
    bool parse = false;
        
    if(obj) {
        PA_Variable params[1], result;
        params[0] = PA_CreateVariable(eVK_Object);
        PA_SetObjectVariable(&params[0], obj);
        result = PA_ExecuteCommandByID(1217, params, 1);
        PA_Unistring json = PA_GetStringVariable(result);
        CUTF16String u16;
        u16 = PA_GetUnistring(&json);
        std::string u8;
        u16_to_u8(u16, u8);
        
        Json::CharReaderBuilder builder;
        Json::CharReader *reader = builder.newCharReader();
        
        std::string errors;
        parse = reader->parse((const char *)u8.c_str(),
                              (const char *)u8.c_str() + u8.size(),
                              jsonValue,
                              &errors);
        delete reader;
    }
        
    return parse;
}

static void json_to_python_object(Json::Value jsonValue, PyObject *obj);

static PyObject *fourd_type_to_python_type(PA_Variable status) {
    
    PyObject *result = NULL;
    
    PA_VariableKind vk = PA_GetVariableKind(status);
    
    switch (vk) {
        case eVK_Blob:
        {
            PA_Handle h = PA_GetBlobHandleVariable(status);
            if(h) {
                result = PyBytes_FromStringAndSize((const char *)PA_LockHandle(h), (Py_ssize_t)PA_GetHandleSize(h));
                PA_UnlockHandle(h);
            }
        }
            break;
        case eVK_Date:
            //TODO: C_DATE
            break;
        case eVK_Real:
            result =  PyFloat_FromDouble(PA_GetRealVariable(status));
            break;
        case eVK_Object:
        {
            PA_ObjectRef o = PA_GetObjectVariable(status);
            Json::Value jsonValue;
            if(fourd_object_to_json(o, &jsonValue)) {
                if(jsonValue.isObject())
                {
                    result = PyDict_New();
                    json_to_python_object(jsonValue, result);
                }
            }
        }
            break;
        case eVK_Boolean:
            if(PA_GetBooleanVariable(status)) {
                Py_RETURN_TRUE;
            }else{
                Py_RETURN_FALSE;
            }
            break;
        case eVK_Longint:
        case eVK_Integer:
        case eVK_Time:
            result = PyLong_FromLong(PA_GetLongintVariable(status));
            break;
        case eVK_Collection:
        {
            PA_CollectionRef c = PA_GetCollectionVariable(status);
            Json::Value jsonValue;
            if(fourd_collection_to_json(c, &jsonValue)) {
                if(jsonValue.isArray())
                {
                    result = PyList_New(0);
                    json_to_python_object(jsonValue, result);
                }
            }
        }
            break;
        case eVK_Picture:
            //TODO: C_PICTURE
            break;
        case eVK_Unistring:
        case eVK_Text:
        {
            PA_Unistring str = PA_GetStringVariable(status);
            CUTF16String u16 = PA_GetUnistring(&str);
            std::string u8;
            u16_to_u8(u16, u8);
            result = PyUnicode_FromStringAndSize(u8.c_str(), (Py_ssize_t)u8.length());
        }
            break;
        case eVK_Null:
            Py_RETURN_NONE;
            break;
        case eVK_Undefined:
        default:
            break;
    }

    return result;
}

static void python_type_to_fourd_type(PyObject *arg, PA_ObjectRef obj);

static void python_list_to_fourd_collection(PyObject *arg, PA_CollectionRef col) {
 
    for(Py_ssize_t i = 0; i < PyList_Size(arg); ++i) {
        PyObject *elem = PyList_GetItem(arg, i);
        if(PyUnicode_Check(elem)) {
            std::string u8 = PyUnicode_AsUTF8AndSize(elem, NULL);
            CUTF16String u16;
            u8_to_u16(u8, u16);
            PA_Variable v = PA_CreateVariable(eVK_Unistring);
            PA_Unistring value = PA_CreateUnistring((PA_Unichar *)u16.c_str());
            PA_SetStringVariable(&v, &value);
            PA_SetCollectionElement(col, PA_GetCollectionLength(col), v);
            continue;
        }
        if(PyLong_Check(elem)) {
            PA_Variable v = PA_CreateVariable(eVK_Longint);
            PA_SetLongintVariable(&v, (PA_long32)PyLong_AsLong(elem));
            PA_SetCollectionElement(col, PA_GetCollectionLength(col), v);
            continue;
        }
        if(PyFloat_Check(elem)) {
            PA_Variable v = PA_CreateVariable(eVK_Real);
            PA_SetRealVariable(&v, PyFloat_AsDouble(elem));
            PA_SetCollectionElement(col, PA_GetCollectionLength(col), v);
            continue;
        }
        if(elem == NULL) {
            PA_Variable v = PA_CreateVariable(eVK_Null);
            PA_SetCollectionElement(col, PA_GetCollectionLength(col), v);
            continue;
        }
        if(PyBool_Check(elem)) {
            PA_Variable v = PA_CreateVariable(eVK_Boolean);
            PA_SetBooleanVariable(&v, (char)PyLong_AsLong(elem));
            PA_SetCollectionElement(col, PA_GetCollectionLength(col), v);
            continue;
        }
        if(PyDict_Check(elem)) {
            PA_ObjectRef prop = PA_CreateObject();
            python_type_to_fourd_type(elem, prop);
            PA_Variable v = PA_CreateVariable(eVK_Object);
            PA_SetObjectVariable(&v, prop);
            PA_SetCollectionElement(col, PA_GetCollectionLength(col), v);
            continue;
        }
        if(PyList_Check(elem)) {
            PA_CollectionRef col2 = PA_CreateCollection();
            python_list_to_fourd_collection(elem, col2);
            PA_Variable v = PA_CreateVariable(eVK_Collection);
            PA_SetCollectionVariable(&v, col2);
            PA_SetCollectionElement(col, PA_GetCollectionLength(col), v);
        }
    }
}
    
static void python_type_to_fourd_type(PyObject *arg, PA_ObjectRef obj) {
    
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    
    while (PyDict_Next(arg, &pos, &key, &value)) {
        const char *u8 = PyUnicode_AsUTF8AndSize(key, NULL);
        if(u8) {
            if(PyUnicode_Check(value)) {
                ob_set_s(obj, u8, PyUnicode_AsUTF8AndSize(value, NULL));
                continue;
            }
            if(PyLong_Check(value)) {
                ob_set_n(obj, u8, PyLong_AsLong(value));
                continue;
            }
            if(PyFloat_Check(value)) {
                ob_set_n(obj, u8, PyFloat_AsDouble(value));
                continue;
            }
            if(value == NULL) {
                ob_set_0(obj, u8);
                continue;
            }
            if(PyBool_Check(value)) {
                ob_set_b(obj, u8, PyLong_AsLong(value));
                continue;
            }
            if(PyDict_Check(value)) {
                PA_ObjectRef prop = PA_CreateObject();
                python_type_to_fourd_type(value, prop);
                ob_set_o(obj, u8, prop);
                continue;
            }
            if(PyList_Check(value)) {
                PA_CollectionRef col = PA_CreateCollection();
                python_list_to_fourd_collection(value, col);
                ob_set_c(obj, u8, col);
            }
        }
    }
}

static PyObject *fourd_call(PyObject *self, PyObject *args) {
    
    PyObject *result = NULL;
    
    char *arg1;
    PyObject *arg2 = NULL;
    
    if (PyArg_ParseTuple(args, "s|O", &arg1, &arg2)) {
        
        PA_long32 method_id = 0;
        std::string u8 = arg1;
        CUTF16String u16;
        u8_to_u16(u8, u16);
        method_id = PA_GetMethodID((PA_Unichar *)u16.c_str());
        int argc = 0;
        if(method_id) {
            
            PA_Variable params[1];
            params[0] = PA_CreateVariable(eVK_Object);
            PA_ObjectRef param1 = PA_CreateObject();
            
            if(arg2) {

                if(PyDict_Check(arg2)) {
                    
                    python_type_to_fourd_type(arg2, param1);

                    argc++;
                }
            }

            PA_SetObjectVariable(&params[0], param1);
            
            PA_Variable status = PA_ExecuteMethodByID(method_id, params, argc);
            
            result = fourd_type_to_python_type(status);
                                    
            PA_ClearVariable(&params[0]);
        }
  
    }
                         
    return result;
}

static PyMethodDef EmbMethods[] = {
    {
        "call",
        fourd_call,
        METH_VARARGS,
        "call 4D project method"
    },
    /* end of list */
    {
        NULL,
        NULL,
        0,
        NULL
    }
};

static PyModuleDef EmbModule = {
    
    PyModuleDef_HEAD_INIT,
    "fourd",
    NULL,
    -1,
    EmbMethods,
    NULL, NULL, NULL, NULL
};

static PyObject *PyInit_fourd(void) {
    
    return PyModule_Create(&EmbModule);
}

static void OnStartup() {

    std::wstring path;
    
    if(getPythonHome(path)) {
 
        //https://docs.python.org/3/c-api/init.html#pre-init-safe
        
		Py_SetPath((wchar_t *)path.c_str());
        Py_SetPythonHome((wchar_t *)path.c_str());
        
        Py_NoSiteFlag = 1;
        Py_IgnoreEnvironmentFlag = 1;
        Py_NoUserSiteDirectory = 1;
                
        PyImport_AppendInittab("fourd", &PyInit_fourd);
        
        Py_InitializeEx(0);

    }
        
}

static void OnExit() {
    Py_FinalizeEx();
}

void PluginMain(PA_long32 selector, PA_PluginParameters params) {
    
	try
	{
        switch(selector)
        {
            case kInitPlugin:
            case kServerInitPlugin:
//                PA_RunInMainProcess((PA_RunInMainProcessProcPtr)OnStartup, NULL);
                OnStartup();
                break;
                
            case kDeinitPlugin:
            case kServerDeinitPlugin:
//                PA_RunInMainProcess((PA_RunInMainProcessProcPtr)OnExit, NULL);
                OnExit();
                break;
                
			// --- python
            
			case 1 :
				python(params);
				break;

        }

	}
	catch(...)
	{

	}
}

#pragma mark -

static void json_to_python_object(Json::Value jsonValue, PyObject *obj) {
    
    if(jsonValue.isObject())
    {
        for(Json::Value::const_iterator it = jsonValue.begin() ; it != jsonValue.end() ; it++)
        {
            JSONCPP_STRING k = it.name();

            if(it->isInt()) {
                PyObject *key = PyUnicode_FromStringAndSize(k.c_str(), (Py_ssize_t)k.length());
                PyObject *value = PyLong_FromLong(it->asInt());
                PyDict_SetItem(obj, key, value);
                continue;
            }
            if(it->isDouble()) {
                PyObject *key = PyUnicode_FromStringAndSize(k.c_str(), (Py_ssize_t)k.length());
                PyObject *value = PyFloat_FromDouble(it->asDouble());
                PyDict_SetItem(obj, key, value);
                continue;
            }
            if(it->isString()) {
                JSONCPP_STRING v = it->asString();
                PyObject *key = PyUnicode_FromStringAndSize(k.c_str(), (Py_ssize_t)k.length());
                PyObject *value = PyUnicode_FromStringAndSize(v.c_str(), (Py_ssize_t)v.length());
                PyDict_SetItem(obj, key, value);
                continue;
            }
            if(it->isBool()) {
                PyObject *key = PyUnicode_FromStringAndSize(k.c_str(), (Py_ssize_t)k.length());
                PyObject *value = it->asBool() ? Py_True : Py_False;
                PyDict_SetItem(obj, key, value);
                continue;
            }
            if(it->isNull()) {
                PyObject *key = PyUnicode_FromStringAndSize(k.c_str(), (Py_ssize_t)k.length());
                PyObject *value = Py_None;
                PyDict_SetItem(obj, key, value);
                continue;
            }
            if(it->isObject()) {
                PyObject *locals = PyDict_New();
                json_to_python_object(*it, locals);
                continue;
            }
            if(it->isArray()) {
                PyObject *locals = PyList_New(0);
                json_to_python_object(*it, locals);
                continue;
            }
        }
    }
    
    if(jsonValue.isArray())
    {
        for(Json::Value::const_iterator i = jsonValue.begin() ; i != jsonValue.end() ; i++)
        {
            Json::Value value = *i;
            if(i->isInt()) {
                PyObject *value = PyLong_FromLong(i->asInt());
                PyList_Append(obj, value);
                continue;
            }
            if(i->isDouble()) {
                PyObject *value = PyFloat_FromDouble(i->asDouble());
                PyList_Append(obj, value);
                continue;
            }
            if(i->isString()) {
                JSONCPP_STRING v = i->asString();
                PyObject *value = PyUnicode_FromStringAndSize(v.c_str(), (Py_ssize_t)v.length());
                PyList_Append(obj, value);
                continue;
            }
            if(i->isBool()) {
                PyObject *value = i->asBool() ? Py_True : Py_False;
                PyList_Append(obj, value);
                continue;
            }
            if(i->isNull()) {
                PyObject *value = Py_None;
                PyList_Append(obj, value);
                continue;
            }
            if(i->isObject()) {
                PyObject *value = PyDict_New();
                json_to_python_object(*i, value);
                PyList_Append(obj, value);
                continue;
            }
            if(i->isArray()) {
                PyObject *value = PyList_New(0);
                json_to_python_object(*i, value);
                PyList_Append(obj, value);
                continue;
            }
        }
    }
}

void python(PA_PluginParameters params) {
    
    PA_ObjectRef status = PA_CreateObject();
    
    ob_set_s(status, L"version", Py_GetVersion());
    ob_set_s(status, L"build", Py_GetBuildInfo());
    ob_set_s(status, L"copyright", Py_GetCopyright());
    ob_set_s(status, L"compiler", Py_GetCompiler());
    
    std::string python;
    
    PA_Unistring *arg1 = PA_GetStringParameter(params, 1);
    if(arg1){
        CUTF16String u16 = (const PA_Unichar *)PA_GetUnistring(arg1);
        u16_to_u8(u16, python);
    }
        
    PyObject *main = PyImport_AddModule("__main__");
    PyObject *globals = PyModule_GetDict(main);
    
    PyObject *locals = PyDict_New();
    
    PA_ObjectRef variables = PA_GetObjectParameter(params, 2);
    if(variables) {
        Json::Value jsonValue;
        if(fourd_object_to_json(variables, &jsonValue)) {
            if(jsonValue.isObject())
            {
                json_to_python_object(jsonValue, locals);
            }
        }
    }
    
    PyObject *ref = NULL;
        
    switch (PA_GetLongParameter(params, 3)) {
        case 1: //python_eval
            ref = PyRun_String(python.c_str(),
                               Py_eval_input,
                               globals,
                               locals);
            break;
            
        case 2: //python_line
            ref = PyRun_String(python.c_str(),
                               Py_single_input,
                               globals,
                               locals);
            break;
            
        default://python_exec
            ref = PyRun_String(python.c_str(),
                               Py_file_input,
                               globals,
                               locals);
            break;
    }
    

    
    Py_DECREF(locals);
    
    if (ref != NULL) {
        PA_ObjectRef returnValue = PA_CreateObject();
        python_type_to_fourd_type(ref, returnValue);
        ob_set_o(status, L"returnValue", returnValue);
    }
    
    if(PyErr_Occurred()) {

        //PyErr_Print() also clears the error indicator so don't use it!
        
        PyObject *errType, *errValue, *errTraceback;
        PyErr_Fetch(&errType, &errValue, &errTraceback);
        
        if(errValue){
            const char *u8 = PyUnicode_AsUTF8AndSize(errValue, NULL);
            ob_set_s(status, L"errorMessage", u8);
        }

        Py_XDECREF(errType);
        Py_XDECREF(errValue);
        Py_XDECREF(errTraceback);

        PyErr_Clear();//need this because a callback returning NULL is technically an error
    }
    
    PA_ReturnObject(params, status);
}

#pragma mark -

static void u16_to_u8(CUTF16String& u16, std::string& u8) {
    
#ifdef _WIN32
    int len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u16.c_str(), u16.length(), NULL, 0, NULL, NULL);
    
    if(len){
        std::vector<uint8_t> buf(len + 1);
        if(WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u16.c_str(), u16.length(), (LPSTR)&buf[0], len, NULL, NULL)){
            u8 = std::string((const char *)&buf[0]);
        }
    }else{
        u8 = std::string((const char *)"");
    }

#else
    CFStringRef str = CFStringCreateWithCharacters(kCFAllocatorDefault, (const UniChar *)u16.c_str(), u16.length());
    if(str){
        size_t size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(str), kCFStringEncodingUTF8) + sizeof(uint8_t);
        std::vector<uint8_t> buf(size);
        CFIndex len = 0;
        CFStringGetBytes(str, CFRangeMake(0, CFStringGetLength(str)), kCFStringEncodingUTF8, 0, true, (UInt8 *)&buf[0], size, &len);
        u8 = std::string((const char *)&buf[0], len);
        CFRelease(str);
    }
#endif
}

static void u8_to_u16(std::string& u8, CUTF16String& u16) {
    
#ifdef _WIN32
    int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8.c_str(), u8.length(), NULL, 0);
    
    if(len){
        std::vector<uint8_t> buf((len + 1) * sizeof(PA_Unichar));
        if(MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)u8.c_str(), u8.length(), (LPWSTR)&buf[0], len)){
            u16 = CUTF16String((const PA_Unichar *)&buf[0]);
        }
    }else{
        u16 = CUTF16String((const PA_Unichar *)L"");
    }
    
#else
    CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)u8.c_str(), u8.length(), kCFStringEncodingUTF8, true);
    if(str){
        CFIndex len = CFStringGetLength(str);
        std::vector<uint8_t> buf((len+1) * sizeof(PA_Unichar));
        CFStringGetCharacters(str, CFRangeMake(0, len), (UniChar *)&buf[0]);
        u16 = CUTF16String((const PA_Unichar *)&buf[0]);
        CFRelease(str);
    }
#endif
}
