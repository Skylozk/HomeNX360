#pragma once
#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
namespace switchu::manager::policy360 {
struct Version {
    std::array<int,5> parts{};
    auto operator<=>(const Version&)const=default;
};
inline std::optional<Version> version(std::string text){
    if(text.size()>64)return {};if(!text.empty() && text[0]=='v')text.erase(0,1);size_t at=0;Version v;
    auto number=[&](int& out){size_t start=at;out=0;while(at<text.size() && text[at]>='0' && text[at]<='9'){if(out>100000)return false;out=out*10+text[at++]-'0';}return at>start;};
    for(int i=0;i<3;++i){if(!number(v.parts[i]))return {};if(i<2 && (at>=text.size() || text[at++]!='.'))return {};}
    if(at==text.size()){v.parts[3]=1;return v;}
    if(text.compare(at,9,"-preview.")!=0)return {};at+=9;if(!number(v.parts[4]) || at!=text.size())return {};return v;
}
inline bool newer(const std::string& a,const std::string& b){const auto x=version(a),y=version(b);return x && (!y || *x>*y);}
inline bool digest(const std::string& h){return h.size()==64 && h.find_first_not_of("0123456789abcdefABCDEF")==std::string::npos;}
inline constexpr const char* previewPaths[]={"switch/HomeNX360/HomeNX360.nro","switch/HomeNX360-preview.nro","switch/HomeNX360.nro","switch/HomeNX360-preview/HomeNX360-preview.nro","switch/HomeNX360/HomeNX360-preview.nro"};
inline constexpr const char* updaterPaths[]={"switch/HomeNX360-Updater/HomeNX360-Updater.nro","switch/HomeNX360-Updater.nro"};
inline bool updaterPath(const std::string& path){for(const char* p:updaterPaths)if(path==p)return true;return false;}
inline bool previewPath(const std::string& path){for(const char* p:previewPaths)if(path==p)return true;return false;}
inline std::string allowedPath(std::string path){
    std::replace(path.begin(),path.end(),'\\','/');
    if(path.empty() || path[0]=='/' || path.find(':')!=std::string::npos)return {};
    size_t start=0;while(start<path.size()){const auto end=path.find('/',start);const auto part=path.substr(start,end==std::string::npos?end:end-start);if(part==".." || part=="." || part.empty())return {};if(end==std::string::npos)break;start=end+1;}
    if(path.rfind("switch/SwitchU/",0)==0 || path=="atmosphere/contents/0100000000001000/exefs.nsp" || path=="config/SwitchU/frontend.txt" || previewPath(path) || updaterPath(path) || path=="switch/HomeNX360/version.txt")return path;
    return {};
}
struct Candidate {std::string version,name,url,sha;uint64_t size=0;};
inline std::optional<Candidate> latest(const nlohmann::json& list){
    if(!list.is_array() || list.size()>100)return {};std::optional<Candidate> result;
    for(const auto& release:list){try{
        if(!release.is_object() || release.value("draft",true))continue;
        const auto tag=release.value("tag_name","");if(!version(tag) || !release.contains("assets") || !release["assets"].is_array())continue;
        for(const auto& asset:release["assets"]){
            if(asset.value("name","")!="HomeNX360-update.zip")continue;
            const auto url=asset.value("browser_download_url","");
            const std::string expected="https://github.com/Skylozk/HomeNX360/releases/download/"+tag+"/HomeNX360-update.zip";if(url!=expected)continue;
            if(!asset.contains("digest") || !asset["digest"].is_string())continue;auto sha=asset["digest"].get<std::string>();if(sha.rfind("sha256:",0)==0)sha.erase(0,7);if(!digest(sha))continue;
            const uint64_t size=asset.value("size",0ULL);if(!size || size>1536ULL*1024*1024)continue;
            if(!result || newer(tag,result->version))result=Candidate{tag,release.value("name",tag),url,sha,size};
        }
    }catch(...){continue;}}
    return result;
}
} // namespace switchu::manager::policy360
