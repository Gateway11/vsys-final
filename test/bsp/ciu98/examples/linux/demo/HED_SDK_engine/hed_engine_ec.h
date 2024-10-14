
#ifndef _HED_ENGINE_EC_H_
#define _HED_ENGINE_EC_H_

#include <openssl/ec.h>
#include <openssl/x509.h>


EVP_PKEY *hed_loadKey(
  ENGINE      *e,
  const char  *key_id,
  UI_METHOD   *ui,
  void        *cb_data
);

#endif // _HED_ENGINE_EC_H_
