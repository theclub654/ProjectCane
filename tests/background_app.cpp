// See background_app.h.

#include "background_app.h"

#include <objc/message.h>
#include <objc/runtime.h>

namespace cane_test
{
namespace
{
using SetPolicyFn = BOOL (*)(id, SEL, long);
SetPolicyFn g_setPolicy = nullptr;

BOOL SetPolicyProhibited(id self, SEL command, long)
{
    return g_setPolicy(self, command, kPolicyProhibited);
}

void IgnoreActivate(id, SEL) {}
void IgnoreActivateIgnoringOtherApps(id, SEL, BOOL) {}

id SharedApplication()
{
    Class application = objc_getClass("NSApplication");
    if (application == nullptr)
        return nullptr;
    return reinterpret_cast<id (*)(Class, SEL)>(objc_msgSend)(application, sel_registerName("sharedApplication"));
}
}

long ActivationPolicy()
{
    const id app = SharedApplication();
    if (app == nullptr)
        return -1;
    return reinterpret_cast<long (*)(id, SEL)>(objc_msgSend)(app, sel_registerName("activationPolicy"));
}

void MakeBackgroundOnly()
{
    Class application = objc_getClass("NSApplication");
    if (application == nullptr || g_setPolicy != nullptr)
        return;
    Method setPolicy = class_getInstanceMethod(application, sel_registerName("setActivationPolicy:"));
    if (setPolicy == nullptr)
        return;
    g_setPolicy = reinterpret_cast<SetPolicyFn>(
        method_setImplementation(setPolicy, reinterpret_cast<IMP>(&SetPolicyProhibited)));
    if (Method m = class_getInstanceMethod(application, sel_registerName("activateIgnoringOtherApps:")))
        method_setImplementation(m, reinterpret_cast<IMP>(&IgnoreActivateIgnoringOtherApps));
    if (Method m = class_getInstanceMethod(application, sel_registerName("activate")))
        method_setImplementation(m, reinterpret_cast<IMP>(&IgnoreActivate));
    reinterpret_cast<BOOL (*)(id, SEL, long)>(objc_msgSend)(
        SharedApplication(), sel_registerName("setActivationPolicy:"), kPolicyProhibited);
}
}
