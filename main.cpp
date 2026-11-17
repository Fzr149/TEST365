#include <android/log.h>
#include <cstring>
#include <hwbinder/Binder.h>
#include <hwbinder/IPCThreadState.h>
#include <hwbinder/Parcel.h>
#include <hwbinder/ProcessState.h>
#include <vector>

#define LOG_TAG "commcenter_compat-dummy"

using android::sp;
using android::status_t;
using android::hardware::BHwBinder;
using android::hardware::IBinder;
using android::hardware::IPCThreadState;
using android::hardware::Parcel;
using android::hardware::ProcessState;

struct HidlString {
    const char* mBuffer;
    uint32_t mSize;
    bool mOwnsBuffer;
};
static_assert(sizeof(HidlString) == 16, "Invalid HidlString ABI size");

struct HidlVec {
    const void* mBuffer;
    uint32_t mSize;
    bool mOwnsBuffer;
};
static_assert(sizeof(HidlVec) == 16, "Invalid HidlVec ABI size");

static void writeHidlString(Parcel* p, const char* str) {
    HidlString hs{str, static_cast<uint32_t>(std::strlen(str)), false};
    size_t parent = 0;
    p->writeBuffer(&hs, sizeof(hs), &parent);
    size_t child = 0;
    p->writeEmbeddedBuffer(hs.mBuffer, hs.mSize + 1, &child, parent, 0);
}

static void writeHidlStringVec(Parcel* p, const std::vector<const char*>& list) {
    std::vector<HidlString> items;
    items.reserve(list.size());
    for (const char* s : list) {
        items.push_back(HidlString{s, static_cast<uint32_t>(std::strlen(s)), false});
    }
    HidlVec vec{items.data(), static_cast<uint32_t>(items.size()), false};
    size_t vecHandle = 0;
    p->writeBuffer(&vec, sizeof(vec), &vecHandle);
    size_t bufHandle = 0;
    p->writeEmbeddedBuffer(items.data(), items.size() * sizeof(HidlString), &bufHandle, vecHandle, 0);
    for (size_t i = 0; i < items.size(); ++i) {
        size_t strHandle = 0;
        p->writeEmbeddedBuffer(items[i].mBuffer, items[i].mSize + 1, &strHandle, bufHandle, i * sizeof(HidlString));
    }
}

class DummyDmtpService : public BHwBinder {
public:
    status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                        uint32_t flags, TransactCallback callback) override {
        (void)callback;
        switch (code) {
            case 256067662: { // interfaceChain
                data.enforceInterface("android.hidl.base@1.0::IBase");
                reply->writeInt32(0); // Status::OK
                writeHidlStringVec(reply, {
                    "vendor.oplus.hardware.communicationcenter_compat@1.1::IDmtpService",
                    "vendor.oplus.hardware.communicationcenter_compat@1.0::IDmtpService",
                    "android.hidl.base@1.0::IBase",
                });
                return 0;
            }
            case 256136003: { // interfaceDescriptor
                data.enforceInterface("android.hidl.base@1.0::IBase");
                reply->writeInt32(0);
                writeHidlString(reply, "vendor.oplus.hardware.communicationcenter_compat@1.1::IDmtpService");
                return 0;
            }
            case 256921159: { // ping
                data.enforceInterface("android.hidl.base@1.0::IBase");
                reply->writeInt32(0);
                return 0;
            }
            default: { // Semua method IDmtpService (code 1, 2, 3, ...)
                if ((flags & 1) == 0 && reply != nullptr) {
                    reply->writeInt32(0); // Status::OK
                    reply->writeInt32(0); // return 0 / false / kosong
                }
                return 0;
            }
        }
    }
};

int main() {
    ProcessState::self()->setThreadPoolMaxThreadCount(1);

    sp<IBinder> service = new DummyDmtpService();
    sp<IBinder> sm = ProcessState::self()->getContextObject(nullptr);

    Parcel data, reply;
    data.writeInterfaceToken("android.hidl.manager@1.0::IServiceManager");
    writeHidlString(&data, "default");
    data.writeStrongBinder(service);

    // Transaction code 2 = IServiceManager::add(string name, interface service)
    status_t err = sm->transact(2, data, &reply);
    if (err != 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Gagal register IDmtpService dummy: %d", err);
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "IDmtpService dummy native aktif");
    IPCThreadState::self()->joinThreadPool();
    return 0;
}
