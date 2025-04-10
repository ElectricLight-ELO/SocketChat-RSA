#include "cFormRequest.h"
#include "authData.h"

string FormRequest::fPubKeys(string targetName, string pK, string mK)
{
    string resp = std::format("<from>null</from>"
        "<to>{}</to>"
        "<type>publKeys</type>"
        "<data>null</data>"
        "<publicKey>{}</publicKey>"
        "<modulus>{}</modulus>", targetName, pK, mK);
    return resp;
}

string FormRequest::fMessage(string to, string data)
{
    string resp = std::format("<from>{}</from>"
        "<to>{}</to>"
        "<type>msg</type>"
        "<data>{}</data>"
        "<publicKey>null</publicKey>"
        "<modulus>null</modulus>", login, to, data);
    return resp;
}
