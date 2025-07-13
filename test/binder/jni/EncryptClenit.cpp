#include <jni.h>
#include <android/binder_ibinder_jni.h>

#include "IEncryptor.h"

std::shared_ptr<IEncryptor> g_spMyService;

class BpEncryptor : public ndk::BpCInterface<IEncryptor>
{
public:
    BpEncryptor(const ndk::SpAIBinder& binder) : BpCInterface(binder){}

    binder_status_t encrypt(char * src, int length, char* out, int &error) override {
        LOGI( "BpEncryptor: encrypt ");
        binder_status_t stat = STATUS_OK;

        AParcel* parcelIn;
        stat = AIBinder_prepareTransaction(asBinder().get(), &parcelIn);
        if (stat != STATUS_OK) return stat;

        stat = AParcel_writeInt32(parcelIn, length);
        if (stat != STATUS_OK) return stat;

        stat = AParcel_writeCharArray(parcelIn, reinterpret_cast<const char16_t *>(src), length);
        if (stat != STATUS_OK) return stat;

        stat = AParcel_writeInt32(parcelIn, length);
        if (stat != STATUS_OK) return stat;

        ndk::ScopedAParcel parcelOut;
        stat = AIBinder_transact(asBinder().get(), TRANSACTION_ENCRYPT, &parcelIn, parcelOut.getR(), 0 /*flags*/);
        if (stat != STATUS_OK) return stat;

        int32_t size = 0;

        stat = AParcel_readInt32(parcelOut.get(), &size);
        if (stat != STATUS_OK) return stat;

        return stat;
    }

    int decrypt(char * src, int length, char* out, int &error) override{
        LOGI( "BpEncryptor: decrypt ");
        ndk::ScopedAParcel parcelOut;
        binder_status_t stat = STATUS_OK;
        AParcel* parcelIn;
        stat = AIBinder_prepareTransaction(asBinder().get(), &parcelIn);
        if (stat != STATUS_OK) return stat;

        stat = AIBinder_transact(asBinder().get(), TRANSACTION_DECRYPT, &parcelIn, parcelOut.getR(), 0 /*flags*/);

        return stat;
    }
};

extern "C" JNIEXPORT void JNICALL
Java_com_sdt_aidl_Native_onServiceConnected(JNIEnv* env, jobject /* this */, jobject binder) {

    AIBinder* pBinder = AIBinder_fromJavaBinder(env, binder);

    const ::ndk::SpAIBinder spBinder(pBinder);
    g_spMyService = ndk::SharedRefBase::make<BpEncryptor>(spBinder);
}

