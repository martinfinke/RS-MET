/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== DebugNotes.txt ==================
static const unsigned char temp_binary_data_0[] =
"For an easy workflow for debugging, build the PluginHost (in the \r\n"
"AudioApplications folder) and set up the visual studio project such that it\r\n"
"runs the just built Plugin Host. For this to work, set on the Property settings\r\n"
"under \"Debugging\" the \"Command\" field to the .exe that results from the build.\r\n"
"On my machine, the path is:\r\n"
"\r\n"
"E:\\Programming\\C++\\RS-MET\\Products\\AudioApplications\\PluginHost\\Builds\\VisualStudio2015\\x64\\Debug\\Plugin Host.exe\r\n"
"\r\n";

const char* DebugNotes_txt = (const char*) temp_binary_data_0;

//================== ToDo.txt ==================
static const unsigned char temp_binary_data_1[] =
"";

const char* ToDo_txt = (const char*) temp_binary_data_1;


const char* getNamedResource (const char*, int&) throw();
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes) throw()
{
    unsigned int hash = 0;
    if (resourceNameUTF8 != 0)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x4358baff:  numBytes = 453; return DebugNotes_txt;
        case 0x80091737:  numBytes = 0; return ToDo_txt;
        default: break;
    }

    numBytes = 0;
    return 0;
}

const char* namedResourceList[] =
{
    "DebugNotes_txt",
    "ToDo_txt"
};

}