#pragma once
#include <cstdint>
#include <openssl/sha.h>
using Result=uint32_t;using s64=int64_t;
constexpr int NifmServiceType_User=0;
inline bool R_FAILED(Result r){return r!=0;}
inline Result nifmInitialize(int){return 0;}inline void nifmExit(){}
inline Result socketInitializeDefault(){return 0;}inline void socketExit(){}
inline void svcSleepThread(s64){}
inline Result fsdevCommitDevice(const char*){return 0;}
constexpr size_t SHA256_HASH_SIZE=32;
struct Sha256Context {SHA256_CTX ctx;};
inline void sha256ContextCreate(Sha256Context* c){SHA256_Init(&c->ctx);}
inline void sha256ContextUpdate(Sha256Context* c,const void* p,size_t n){SHA256_Update(&c->ctx,p,n);}
inline void sha256ContextGetHash(Sha256Context* c,void* p){auto copy=c->ctx;SHA256_Final(static_cast<unsigned char*>(p),&copy);}
namespace switchu {struct FileLog {template<class...T>static void log(const char*,T...){}};}
