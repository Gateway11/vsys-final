#ifndef IVOICE_SERVICE_H
#define IVOICE_SERVICE_H

#include "IEncryptor.h"

class BnEncryptServer : public ndk::BnCInterface<IEncryptor>{
public:
    ndk::SpAIBinder createBinder() override;

    binder_status_t encrypt(char *src, int length, char *out, int &error) override;

    binder_status_t decrypt(char *src, int length, char *out, int &error) override;
};

 
#endif
