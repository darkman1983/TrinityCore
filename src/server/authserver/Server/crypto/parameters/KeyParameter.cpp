/**
 * Original code from: http://www.bouncycastle.org/csharp/
 *
 * Ported from C# to C++ by kizura, 2012-Jan
 *
 * License: http://www.gnu.org/licenses/gpl.html
 *
 */
#include "../IDigest.h"

#include "KeyParameter.h"
#include "../util/Array.h"
#include "../util/Pack.h"

#include "KeyParameter.h"

using namespace Org_BouncyCastle_Crypto_Utilities;

using namespace Org_BouncyCastle_Crypto;

namespace Org_BouncyCastle_Crypto_Parameters {

KeyParameter::KeyParameter(uint8* pKey, int keyLen) {
    /*
    if (key == NULL)
        throw new ArgumentNullException("key");
    */
    //this.key = (uint8*) key.Clone();
    key_Length = keyLen;
    this->key = new uint8[keyLen];
    Array::Copy(pKey, 0, key, 0, (uint32)keyLen );
}

KeyParameter::KeyParameter(uint8* key, int keyOff, int keyLen) {
    /*
    if (key == NULL)
        throw new ArgumentNullException("key");
    if (keyOff < 0 || keyOff > key.Length)
        throw new ArgumentOutOfRangeException("keyOff");
    if (keyLen < 0 || (keyOff + keyLen) > key.Length)
        throw new ArgumentOutOfRangeException("keyLen");
    */
    key_Length = keyLen;
    this->key = new uint8[keyLen];
    Array::Copy(key, keyOff, this->key, 0, keyLen);
}

uint8* KeyParameter::GetKey() {
    uint8* retKey = new uint8[key_Length];
    Array::Copy(this->key, 0, retKey, 0, (uint32)key_Length );
    return retKey;
    //return (uint8*) key.Clone();
}
uint32 KeyParameter::GetKeyLength() {
    return key_Length;
}
}
