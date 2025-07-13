#include <android/binder_ibinder.h>
#include <android/binder_interface_utils.h>
#include <android/binder_auto_utils.h>

#define LOGI(...) ({})

enum {
    TRANSACTION_ENCRYPT,
    TRANSACTION_DECRYPT,
};
 
class IEncryptor : public ndk::ICInterface
{
 
public:
    virtual binder_status_t encrypt(char * src, int length, char* out, int &error) = 0;
 
    virtual binder_status_t decrypt(char * src, int length, char* out, int &error) = 0;
};
