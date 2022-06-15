#include "MessageDefine.h"

namespace cli
{

    text get_message_desc(CrInt32u code)
    {
        text retMsg = TEXT(""); // return value
        try
        {
            std::unordered_map<CrInt32u, text>::const_iterator itr; // use twice

            // details
            text str2 = TEXT("");
            itr = map_ERR_DETAIL.find(code);
            if (itr != map_ERR_DETAIL.end()) {
                str2.append(itr->second);
            }
            else
            {
                return retMsg; // return the blank
            }

            // category
            text str1 = TEXT("-"); // default
            CrInt32u maskCat = code & 0xFF00;
            itr = map_ERR_CAT.find(maskCat);
            if (itr != map_ERR_CAT.end())
            {
                str1.clear(); // remove hyphen
                str1.append(itr->second);
            }

            // result
            retMsg.append(TEXT("[CAT:")).append(str1).append(TEXT("] [DETAILS:")).append(str2).append(TEXT("]"));

        }
        catch (std::out_of_range) 
        {
            //tout << std::endl << "Exception Error: 0x" << std::hex << error << std::dec;
        }
        return retMsg;
    }

} // namespace cli
