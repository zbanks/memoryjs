#include <napi.h>
#include "module.h"
#include "process.h"
#include "memoryjs.h"
#include "memory.h"

process Process;
// module Module;
memory Memory;

struct Vector3 {
  float x, y, z;
};

struct Vector4 {
  float w, x, y, z;
};

Napi::Value openProcess(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() != 1 && args.Length() != 2) {
    Napi::Error::New(env, "requires 1 argument, or 2 arguments if a callback is being used").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!args[0].IsString() && !args[0].IsNumber()) {
    Napi::Error::New(env, "first argument must be a string or a number").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (args.Length() == 2 && !args[1].IsFunction()) {
    Napi::Error::New(env, "second argument must be a function").ThrowAsJavaScriptException();
    return env.Null();
  }

  // Define error message that may be set by the function that opens the process
  char* errorMessage = "";

  process::Pair pair;

  if (args[0].IsString()) {
    std::string processName(args[0].As<Napi::String>().Utf8Value());
    pair = Process.openProcess(processName.c_str(), &errorMessage);

    // In case it failed to open, let's keep retrying
    // while(!strcmp(process.szExeFile, "")) {
    //   process = Process.openProcess((char*) *(processName), &errorMessage);
    // };
  }

  if (args[0].IsNumber()) {
    pair = Process.openProcess(args[0].As<Napi::Number>().Uint32Value(), &errorMessage);

    // In case it failed to open, let's keep retrying
    // while(!strcmp(process.szExeFile, "")) {
    //   process = Process.openProcess(info[0].As<Napi::Number>().Uint32Value(), &errorMessage);
    // };
  }

  // If an error message was returned from the function that opens the process, throw the error.
  // Only throw an error if there is no callback (if there's a callback, the error is passed there).
  if (strcmp(errorMessage, "") && args.Length() != 2) {
    Napi::Error::New(env, errorMessage).ThrowAsJavaScriptException();
    return env.Null();
  }

  // Create a v8 Object (JSON) to store the process information
  Napi::Object processInfo = Napi::Object::New(env);

  processInfo.Set(Napi::String::New(env, "dwSize"), Napi::Value::From(env, (int)pair.process.dwSize));
  processInfo.Set(Napi::String::New(env, "th32ProcessID"), Napi::Value::From(env, (int)pair.process.th32ProcessID));
  processInfo.Set(Napi::String::New(env, "cntThreads"), Napi::Value::From(env, (int)pair.process.cntThreads));
  processInfo.Set(Napi::String::New(env, "th32ParentProcessID"), Napi::Value::From(env, (int)pair.process.th32ParentProcessID));
  processInfo.Set(Napi::String::New(env, "pcPriClassBase"), Napi::Value::From(env, (int)pair.process.pcPriClassBase));
  processInfo.Set(Napi::String::New(env, "szExeFile"), Napi::String::New(env, pair.process.szExeFile));
  processInfo.Set(Napi::String::New(env, "handle"), Napi::Value::From(env, (uintptr_t)pair.handle));

  DWORD64 base = module::getBaseAddress(pair.process.szExeFile, pair.process.th32ProcessID);
  processInfo.Set(Napi::String::New(env, "modBaseAddr"), Napi::Value::From(env, (uintptr_t)base));

  // openProcess can either take one argument or can take
  // two arguments for asychronous use (second argument is the callback)
  if (args.Length() == 2) {
    // Callback to let the user handle with the information
    Napi::Function callback = args[1].As<Napi::Function>();
    callback.Call(env.Global(), { Napi::String::New(env, errorMessage), processInfo });
    return env.Null();
  } else {
    // return JSON
    return processInfo;
  }
}

Napi::Value closeProcess(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() != 1) {
    Napi::Error::New(env, "requires 1 argument").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!args[0].IsNumber()) {
    Napi::Error::New(env, "first argument must be a number").ThrowAsJavaScriptException();
    return env.Null();
  }

  Process.closeProcess((HANDLE)args[0].As<Napi::Number>().Int64Value());
  return env.Null();
}

Napi::Value getProcesses(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() > 1) {
    Napi::Error::New(env, "requires either 0 arguments or 1 argument if a callback is being used").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (args.Length() == 1 && !args[0].IsFunction()) {
    Napi::Error::New(env, "first argument must be a function").ThrowAsJavaScriptException();
    return env.Null();
  }

  // Define error message that may be set by the function that gets the processes
  char* errorMessage = "";

  std::vector<PROCESSENTRY32> processEntries = Process.getProcesses(&errorMessage);

  // If an error message was returned from the function that gets the processes, throw the error.
  // Only throw an error if there is no callback (if there's a callback, the error is passed there).
  if (strcmp(errorMessage, "") && args.Length() != 1) {
    Napi::Error::New(env, errorMessage).ThrowAsJavaScriptException();
    return env.Null();
  }

  // Creates v8 array with the size being that of the processEntries vector processes is an array of JavaScript objects
  Napi::Array processes = Napi::Array::New(env, processEntries.size());

  // Loop over all processes found
  for (std::vector<PROCESSENTRY32>::size_type i = 0; i != processEntries.size(); i++) {
    // Create a v8 object to store the current process' information
    Napi::Object process = Napi::Object::New(env);

    process.Set(Napi::String::New(env, "cntThreads"), Napi::Value::From(env, (int)processEntries[i].cntThreads));
    process.Set(Napi::String::New(env, "szExeFile"), Napi::String::New(env, processEntries[i].szExeFile));
    process.Set(Napi::String::New(env, "th32ProcessID"), Napi::Value::From(env, (int)processEntries[i].th32ProcessID));
    process.Set(Napi::String::New(env, "th32ParentProcessID"), Napi::Value::From(env, (int)processEntries[i].th32ParentProcessID));
    process.Set(Napi::String::New(env, "pcPriClassBase"), Napi::Value::From(env, (int)processEntries[i].pcPriClassBase));

    // Push the object to the array
    processes.Set(i, process);
  }

  /* getProcesses can either take no arguments or one argument
     one argument is for asychronous use (the callback) */
  if (args.Length() == 1) {
    // Callback to let the user handle with the information
    Napi::Function callback = args[0].As<Napi::Function>();
    callback.Call(env.Global(), { Napi::String::New(env, errorMessage), processes });
    return env.Null();
  } else {
    // return JSON
    return processes;
  }
}

Napi::Value getModules(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() != 1 && args.Length() != 2) {
    Napi::Error::New(env, "requires 1 argument, or 2 arguments if a callback is being used").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!args[0].IsNumber()) {
    Napi::Error::New(env, "first argument must be a number").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (args.Length() == 2 && !args[1].IsFunction()) {
    Napi::Error::New(env, "first argument must be a number, second argument must be a function").ThrowAsJavaScriptException();
    return env.Null();
  }

  // Define error message that may be set by the function that gets the modules
  char* errorMessage = "";

  std::vector<MODULEENTRY32> moduleEntries = module::getModules(args[0].As<Napi::Number>().Int32Value(), &errorMessage);

  // If an error message was returned from the function getting the modules, throw the error.
  // Only throw an error if there is no callback (if there's a callback, the error is passed there).
  if (strcmp(errorMessage, "") && args.Length() != 2) {
    Napi::Error::New(env, errorMessage).ThrowAsJavaScriptException();
    return env.Null();
  }

  // Creates v8 array with the size being that of the moduleEntries vector
  // modules is an array of JavaScript objects
  Napi::Array modules = Napi::Array::New(env, moduleEntries.size());

  // Loop over all modules found
  for (std::vector<MODULEENTRY32>::size_type i = 0; i != moduleEntries.size(); i++) {
    //  Create a v8 object to store the current module's information
    Napi::Object module = Napi::Object::New(env);

    module.Set(Napi::String::New(env, "modBaseAddr"), Napi::Value::From(env, (uintptr_t)moduleEntries[i].modBaseAddr));
    module.Set(Napi::String::New(env, "modBaseSize"), Napi::Value::From(env, (int)moduleEntries[i].modBaseSize));
    module.Set(Napi::String::New(env, "szExePath"), Napi::String::New(env, moduleEntries[i].szExePath));
    module.Set(Napi::String::New(env, "szModule"), Napi::String::New(env, moduleEntries[i].szModule));
    module.Set(Napi::String::New(env, "th32ModuleID"), Napi::Value::From(env, (int)moduleEntries[i].th32ProcessID));

    // Push the object to the array
    modules.Set(i, module);
  }

  // getModules can either take one argument or two arguments
  // one/two arguments is for asychronous use (the callback)
  if (args.Length() == 2) {
    // Callback to let the user handle with the information
    Napi::Function callback = args[1].As<Napi::Function>();
    callback.Call(env.Global(), { Napi::String::New(env, errorMessage), modules });
    return env.Null();
  } else {
    // return JSON
    return modules;
  }
}

Napi::Value findModule(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() != 1 && args.Length() != 2 && args.Length() != 3) {
    Napi::Error::New(env, "requires 1 argument, 2 arguments, or 3 arguments if a callback is being used").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!args[0].IsString() && !args[1].IsNumber()) {
    Napi::Error::New(env, "first argument must be a string, second argument must be a number").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (args.Length() == 3 && !args[2].IsFunction()) {
    Napi::Error::New(env, "third argument must be a function").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string moduleName(args[0].As<Napi::String>().Utf8Value());

  // Define error message that may be set by the function that gets the modules
  char* errorMessage = "";

  MODULEENTRY32 module = module::findModule(moduleName.c_str(), args[1].As<Napi::Number>().Int32Value(), &errorMessage);

  // If an error message was returned from the function getting the module, throw the error.
  // Only throw an error if there is no callback (if there's a callback, the error is passed there).
  if (strcmp(errorMessage, "") && args.Length() != 3) {
    Napi::Error::New(env, errorMessage).ThrowAsJavaScriptException();
    return env.Null();
  }

  // In case it failed to open, let's keep retrying
  while (!strcmp(module.szExePath, "")) {
    module = module::findModule(moduleName.c_str(), args[1].As<Napi::Number>().Int32Value(), &errorMessage);
  };

  // Create a v8 Object (JSON) to store the process information
  Napi::Object moduleInfo = Napi::Object::New(env);

  moduleInfo.Set(Napi::String::New(env, "modBaseAddr"), Napi::Value::From(env, (uintptr_t)module.modBaseAddr));
  moduleInfo.Set(Napi::String::New(env, "modBaseSize"), Napi::Value::From(env, (int)module.modBaseSize));
  moduleInfo.Set(Napi::String::New(env, "szExePath"), Napi::String::New(env, module.szExePath));
  moduleInfo.Set(Napi::String::New(env, "szModule"), Napi::String::New(env, module.szModule));
  moduleInfo.Set(Napi::String::New(env, "th32ProcessID"), Napi::Value::From(env, (int)module.th32ProcessID));
  moduleInfo.Set(Napi::String::New(env, "hModule"), Napi::Value::From(env, (uintptr_t)module.hModule));

  // findModule can either take one or two arguments,
  // three arguments for asychronous use (third argument is the callback)
  if (args.Length() == 3) {
    // Callback to let the user handle with the information
    Napi::Function callback = args[2].As<Napi::Function>();
    callback.Call(env.Global(), { Napi::String::New(env, errorMessage), moduleInfo });
    return env.Null();
  } else {
    // return JSON
    return moduleInfo;
  }
}

Napi::Value readMemory(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() != 3 && args.Length() != 4) {
    Napi::Error::New(env, "requires 3 arguments, or 4 arguments if a callback is being used").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!args[0].IsNumber() && !args[1].IsNumber() && !args[2].IsString()) {
    Napi::Error::New(env, "first and second argument must be a number, third argument must be a string").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (args.Length() == 4 && !args[3].IsFunction()) {
    Napi::Error::New(env, "fourth argument must be a function").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string dataTypeArg(args[2].As<Napi::String>().Utf8Value());
  const char* dataType = dataTypeArg.c_str();

  // Define the error message that will be set if no data type is recognised
  std::string errorMessage;
  Napi::Value retVal = env.Null();

  HANDLE handle = (HANDLE)args[0].As<Napi::Number>().Int64Value();
  DWORD64 address = args[1].As<Napi::Number>().Int64Value();

  if (!strcmp(dataType, "byte")) {

    unsigned char result = Memory.readMemory<unsigned char>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "int")) {

    int result = Memory.readMemory<int>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "int32")) {

    int32_t result = Memory.readMemory<int32_t>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "uint32")) {

    uint32_t result = Memory.readMemory<uint32_t>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "int64")) {

    int64_t result = Memory.readMemory<int64_t>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "uint64")) {

    uint64_t result = Memory.readMemory<uint64_t>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "dword")) {

    DWORD result = Memory.readMemory<DWORD>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "short")) {

    short result = Memory.readMemory<short>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "long")) {

    long result = Memory.readMemory<long>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "float")) {

    float result = Memory.readMemory<float>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "double")) {

    double result = Memory.readMemory<double>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "ptr") || !strcmp(dataType, "pointer")) {

    intptr_t result = Memory.readMemory<intptr_t>(handle, address);
    retVal = Napi::Value::From(env, result);

  } else if (!strcmp(dataType, "bool") || !strcmp(dataType, "boolean")) {

    bool result = Memory.readMemory<bool>(handle, address);
    retVal = Napi::Boolean::New(env, result);

  } else if (!strcmp(dataType, "string") || !strcmp(dataType, "str")) {

    std::vector<char> chars;
    int offset = 0x0;
    while (true) {
      char c = Memory.readChar(handle, address + offset);
      chars.push_back(c);

      // break at 1 million chars
      if (offset == (sizeof(char) * 1000000)) {
        chars.clear();
        break;
      }

      // break at terminator (end of string)
      if (c == '\0') {
        break;
      }

	  // go to next char
      offset += sizeof(char);
    }

    if (chars.size() == 0) {

      if (args.Length() == 4) errorMessage = "unable to read string (no null-terminator found after 1 million chars)";
      else
      {
        Napi::Error::New(env, "unable to read string (no null-terminator found after 1 million chars)").ThrowAsJavaScriptException();
        return env.Null();
      }

    } else {
      // vector -> string
      std::string str(chars.begin(), chars.end());

      retVal = Napi::String::New(env, str.c_str());
    }

  } else if (!strcmp(dataType, "vector3") || !strcmp(dataType, "vec3")) {

    Vector3 result = Memory.readMemory<Vector3>(handle, address);
    Napi::Object moduleInfo = Napi::Object::New(env);
    moduleInfo.Set(Napi::String::New(env, "x"), Napi::Value::From(env, result.x));
    moduleInfo.Set(Napi::String::New(env, "y"), Napi::Value::From(env, result.y));
    moduleInfo.Set(Napi::String::New(env, "z"), Napi::Value::From(env, result.z));

    retVal = moduleInfo;
  } else if (!strcmp(dataType, "vector4") || !strcmp(dataType, "vec4")) {

    Vector4 result = Memory.readMemory<Vector4>(handle, address);
    Napi::Object moduleInfo = Napi::Object::New(env);
    moduleInfo.Set(Napi::String::New(env, "w"), Napi::Value::From(env, result.w));
    moduleInfo.Set(Napi::String::New(env, "x"), Napi::Value::From(env, result.x));
    moduleInfo.Set(Napi::String::New(env, "y"), Napi::Value::From(env, result.y));
    moduleInfo.Set(Napi::String::New(env, "z"), Napi::Value::From(env, result.z));

    retVal = moduleInfo;
  } else {

    if (args.Length() == 4) errorMessage = "unexpected data type";
    else
    {
      Napi::Error::New(env, "unexpected data type").ThrowAsJavaScriptException();
      return env.Null();
    }
  }

  if (args.Length() == 4)
  {
    Napi::Function callback = args[3].As<Napi::Function>();
    if (!errorMessage.empty())
      callback.Call(env.Global(), { Napi::String::New(env, errorMessage) });
    else
      callback.Call(env.Global(), { Napi::String::New(env, ""), retVal });
    return env.Null();
  }
  else
  {
    return retVal;
  }
}

Napi::Value readBuffer(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env();

  if (args.Length() != 3 && args.Length() != 4) {
    Napi::Error::New(env, "requires 3 arguments, or 4 arguments if a callback is being used").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!args[0].IsNumber() && !args[1].IsNumber() && !args[2].IsNumber()) {
    Napi::Error::New(env, "first, second and third arguments must be a number").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (args.Length() == 4 && !args[3].IsFunction()) {
    Napi::Error::New(env, "fourth argument must be a function").ThrowAsJavaScriptException();
    return env.Null();
  }

  HANDLE handle = (HANDLE)args[0].As<Napi::Number>().Int64Value();
  DWORD64 address = args[1].As<Napi::Number>().Int64Value();
  SIZE_T size = args[2].As<Napi::Number>().Int64Value();
  char* data = Memory.readBuffer(handle, address, size);

  Napi::Buffer<char> buffer = Napi::Buffer<char>::New(env, data, size);

  if (args.Length() == 4) {
    Napi::Function callback = args[3].As<Napi::Function>();
    callback.Call(env.Global(), { Napi::String::New(env, ""), buffer });
    return env.Null();
  } else {
    return buffer;
  }
}


// https://stackoverflow.com/a/17387176
std::string GetLastErrorToString() {
  DWORD errorMessageID = ::GetLastError();

  // No error message, return empty string
  if(errorMessageID == 0) {
    return std::string();
  }

  LPSTR messageBuffer = nullptr;

  size_t size = FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    errorMessageID,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPSTR)&messageBuffer,
    0,
    NULL
  );

  std::string message(messageBuffer, size);

  // Free the buffer
  LocalFree(messageBuffer);
  return message;
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "openProcess"), Napi::Function::New(env, openProcess));
  exports.Set(Napi::String::New(env, "closeProcess"), Napi::Function::New(env, closeProcess));
  exports.Set(Napi::String::New(env, "getProcesses"), Napi::Function::New(env, getProcesses));
  exports.Set(Napi::String::New(env, "getModules"), Napi::Function::New(env, getModules));
  exports.Set(Napi::String::New(env, "findModule"), Napi::Function::New(env, findModule));
  exports.Set(Napi::String::New(env, "readMemory"), Napi::Function::New(env, readMemory));
  exports.Set(Napi::String::New(env, "readBuffer"), Napi::Function::New(env, readBuffer));
  return exports;
}

NODE_API_MODULE(memoryjs, init)
