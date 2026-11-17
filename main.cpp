#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <hwbinder/Binder.h>
#include <hwbinder/IPCThreadState.h>
#include <hwbinder/Parcel.h>
#include <utils/Log.h>

using android::sp;
using android::status_t;
using android::hardware::BHwBinder;
using android::hardware::configureRpcThreadpool;
using android::hardware::hidl_string;
using android::hardware::hidl_vec;
using android::hardware::IBinder;
using android::hardware::IPCThreadState;
using android::hardware::joinRpcThreadpool;
using android::hardware::Parcel;

class DummyDmtpService : public BHwBinder {
public:
    status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                        uint32_t flags, TransactCallback callback) override {
        (void)callback;
        switch (code) {
            case 256067662: { // interfaceChain
                data.enforceInterface("android.hidl.base@1.0::IBase");
                reply->writeInt32(0); // Status::OK
                std::vector<hidl_string> chain = {
                    "vendor.oplus.hardware.communicationcenter_compat@1.1::IDmtpService",
                    "vendor.oplus.hardware.communicationcenter_compat@1.0::IDmtpService",
                    "android.hidl.base@1.0::IBase",
                };
                hidl_vec<hidl_string> vec(chain);
                size_t parent;
                reply->writeBuffer(&vec, sizeof(vec), &parent);
                vec.writeEmbeddedToParcel(reply, parent, 0);
                return 0;
            }
            case 256136003: { // interfaceDescriptor
                data.enforceInterface("android.hidl.base@1.0::IBase");
                reply->writeInt32(0);
                hidl_string desc("vendor.oplus.hardware.communicationcenter_compat@1.1::IDmtpService");
                size_t parent;
                reply->writeBuffer(&desc, sizeof(desc), &parent);
                desc.writeEmbeddedToParcel(reply, parent, 0);
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
    configureRpcThreadpool(1, true);

    sp<IBinder> service = new DummyDmtpService();
    sp<IBinder> sm = IPCThreadState::self()->getContextObject(nullptr);

    Parcel data, reply;
    data.writeInterfaceToken("android.hidl.manager@1.0::IServiceManager");
    hidl_string instance("default");
    size_t parent;
    data.writeBuffer(&instance, sizeof(instance), &parent);
    instance.writeEmbeddedToParcel(&data, parent, 0);
    data.writeStrongBinder(service);

    status_t err = sm->transact(1 /* add */, data, &reply);
    if (err != 0) {
        ALOGE("Gagal register IDmtpService dummy: %d", err);
        return 1;
    }

    ALOGI("IDmtpService dummy native aktif");
    joinRpcThreadpool();
    return 0;
}
