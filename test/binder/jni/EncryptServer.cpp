#include "EncryptServer.h"
#include <iostream>

binder_status_t IEncryptor_Class_onTransact(AIBinder* binder, transaction_code_t code,
                                            const AParcel* in, AParcel* out){
    LOGI( "IEncryptor_Class_onTransact  ");
    binder_status_t stat = STATUS_FAILED_TRANSACTION;
 
    uid_t uid = AIBinder_getCallingUid();
    pid_t pid = AIBinder_getCallingPid();
 
    std::shared_ptr<IEncryptor> cipher = std::static_pointer_cast<IEncryptor>(ndk::ICInterface::asInterface(binder));
    if(cipher != nullptr){
        std::cout << "Hello world!!!" << std::endl;
    }
 
    switch (code) {
        case TRANSACTION_ENCRYPT: {
            int32_t valueIn;
            int32_t valueOut;
            stat = AParcel_readInt32(in, &valueIn);
            if (stat != STATUS_OK) break;
            int error;
            stat = cipher->encrypt(nullptr, 0, nullptr, error);
            if (stat != STATUS_OK) break;
            stat = AParcel_writeInt32(out, valueOut);
            break;
        }
        case TRANSACTION_DECRYPT: {
            int error;
            stat = cipher->decrypt(nullptr, 0, nullptr, error);
            break;
        }
    }
    return stat;
}

const char* ENCRYPTOR_DESCRIPTOR = "com.sdt.aidl.IAidl";
 
ndk::SpAIBinder BnEncryptServer::createBinder() {
    LOGI( "BnEncryptServer::createBinder  ");
    AIBinder_Class* IEncryptor = defineClass(ENCRYPTOR_DESCRIPTOR, IEncryptor_Class_onTransact);
    LOGI( "BnEncryptServer::createBinder define class ");
    AIBinder *binder = AIBinder_new(IEncryptor, static_cast<void*>(this));
    LOGI( "BnEncryptServer::createBinder AIBinder_new ");
    return ndk::SpAIBinder(binder);
}
 
binder_status_t BnEncryptServer::encrypt(char *src, int length, char *out, int &error){
    LOGI( "BnEncryptServer::encrypt  ");
    return STATUS_OK;
}
 
binder_status_t BnEncryptServer::decrypt(char *src, int length, char *out, int &error){
    LOGI( "BnEncryptServer::decrypt  ");
    return STATUS_OK;
}
