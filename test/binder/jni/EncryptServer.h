#ifndef IVOICE_SERVICE_H
#define IVOICE_SERVICE_H

#include "IEncryptor.h"

#define LOGI(...) ({})

class BnEncryptServer : public ndk::BnCInterface<IEncryptor>{
public:
    ndk::SpAIBinder createBinder() override;

    binder_status_t encrypt(char *src, int length, char *out, int &error) override;

    binder_status_t decrypt(char *src, int length, char *out, int &error) override;
};

class BpEncryptor : public ndk::BpCInterface<IEncryptor>
{
public:
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
 
#endif
