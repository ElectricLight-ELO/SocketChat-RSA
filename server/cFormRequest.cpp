#include "cFormRequest.h"
#include "Help.h"

string FormRequest::fPubKeys(string from, string pK, string mK)
{
    string resp = format("<from>{}</from>"
        "<to>null</to>"
        "<type>publKeys</type>"
        "<data>null</data>"
        "<publicKey>{}</publicKey>"
        "<modulus>{}</modulus>", from, pK, mK);
    return resp;
}

string FormRequest::fMessage(string from, string to, string data)
{
    string resp = format("<from>{}</from>"
        "<to>{}</to>"
        "<type>msg</type>"
        "<data>{}</data>"
        "<publicKey>null</publicKey>"
        "<modulus>null</modulus>", from, to, data);
    return resp;
}
