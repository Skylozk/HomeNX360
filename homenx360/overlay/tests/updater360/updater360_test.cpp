#define HOMENX360_UPDATER_HOST_TEST
#define HOMENX_BUILD "v0.8.1-preview.1"
#include "../../projects/homenx360-updater/ReleaseUpdater360.cpp"
#include <cassert>
#include <iostream>
using namespace switchu::manager;
static void put(const std::filesystem::path& path,const std::string& data){std::filesystem::create_directories(path.parent_path());std::ofstream(path,std::ios::binary)<<data;}
static std::string get(const std::filesystem::path& path){std::ifstream in(path,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
int main(int argc,char**argv){
    assert(argc==2);const auto fixtures=std::filesystem::absolute(argv[1]);const auto cwd=std::filesystem::current_path();const auto root=std::filesystem::temp_directory_path()/"homenx360-updater-tests";std::filesystem::create_directories(root);std::filesystem::current_path(root);
    assert(policy360::newer("v0.9.0-preview.1","v0.8.9"));assert(policy360::newer("v0.8.1","v0.8.1-preview.99"));assert(!policy360::newer("v0.8.1-preview.99","v0.8.1"));assert(policy360::newer("v0.8.1-preview.10","v0.8.1-preview.9"));assert(!policy360::version("v1.2.3/../../bad"));assert(!policy360::version("latest"));
    for(const char* path:{"../escape","switch/SwitchU/../../config/CardsNX/settings.json","sdmc:/switch/SwitchU/x","/switch/SwitchU/x","config/CardsNX/settings.json","config/HomeNX360/account.json","atmosphere/contents/0100000000000001/exefs.nsp"})assert(safeRelativePath(path).empty());
    assert(!safeRelativePath("switch/SwitchU/cards/avatars/catalog/index.avc").empty());assert(!safeRelativePath("config/SwitchU/frontend.txt").empty());assert(policy360::digest(std::string(64,'a')));assert(!policy360::digest(std::string(64,'z')));
    auto candidate=[](const std::string& tag){return nlohmann::json{{"tag_name",tag},{"draft",false},{"prerelease",true},{"assets",nlohmann::json::array({{{"name","HomeNX360-update.zip"},{"browser_download_url","https://github.com/Skylozk/HomeNX360/releases/download/"+tag+"/HomeNX360-update.zip"},{"digest","sha256:"+std::string(64,'a')},{"size",512}}})}};};
    auto list=nlohmann::json::array({candidate("v0.8.1-preview.9"),candidate("v0.8.1-preview.10"),candidate("v0.8.1")});assert(policy360::latest(list)->version=="v0.8.1");list[2]["draft"]=true;assert(policy360::latest(list)->version=="v0.8.1-preview.10");list[1]["assets"][0]["browser_download_url"]="https://example.com/update.zip";assert(policy360::latest(list)->version=="v0.8.1-preview.9");list[0]["assets"][0]["digest"]=nullptr;assert(!policy360::latest(list));
    put("sdmc:/config/CardsNX/settings.json","saved avatars");put("sdmc:/switch/HomeNX360-preview.nro","old-preview");put("sdmc:/switch/HomeNX360-Updater.nro","old-updater");std::filesystem::create_directories(kWorkRoot);std::filesystem::copy_file(fixtures/"valid.zip",kArchivePath,std::filesystem::copy_options::overwrite_existing);
    std::atomic<float> progress{0};const auto zipped=get(kArchivePath);Sha256Context cacheHash{};sha256ContextCreate(&cacheHash);sha256ContextUpdate(&cacheHash,zipped.data(),zipped.size());std::array<uint8_t,SHA256_HASH_SIZE> digestBytes{};sha256ContextGetHash(&cacheHash,digestBytes.data());ReleaseInfo cached;cached.sha256=hexDigest(digestBytes);cached.downloadSize=zipped.size();assert(downloadLocked(cached,progress)==cached.sha256 && progress.load()==1);
    auto files=extractArchive(progress,0,1);assert(progress.load()==1);assert(std::set<std::string>(files.begin(),files.end()).size()==files.size());assert(std::find(files.begin(),files.end(),policy360::previewPaths[0])!=files.end());assert(std::find(files.begin(),files.end(),"switch/HomeNX360-preview.nro")!=files.end());assert(!std::filesystem::exists(root/"escaped.txt"));assert(get("sdmc:/config/CardsNX/settings.json")=="saved avatars");
    // The Switch report failed while copying cardsnx.nro to a staged legacy
    // preview path. Reusing that destination must truncate old bytes safely.
    const auto stagedPreview=std::filesystem::path(kStagingRoot)/"switch/HomeNX360-preview.nro";
    const auto sourcePreview=std::filesystem::path(kStagingRoot)/"switch/SwitchU/bin/cardsnx.nro";
    put(stagedPreview,std::string(400000,'x'));copyFileContents(sourcePreview,stagedPreview);assert(get(stagedPreview)=="new-preview");
    copyFileContents(sourcePreview,stagedPreview);assert(get(stagedPreview)=="new-preview");
    bool samePath=false;try{copyFileContents(sourcePreview,sourcePreview);}catch(...){samePath=true;}assert(samePath && get(sourcePreview)=="new-preview");
    installFiles(files,false,progress,0,1);assert(get("sdmc:/switch/HomeNX360-preview.nro")=="new-preview");assert(get("sdmc:/switch/HomeNX360/HomeNX360.nro")=="new-preview");assert(get("sdmc:/switch/HomeNX360-Updater.nro")=="new-updater");assert(ReleaseUpdater::installedVersion()=="v0.8.1-preview.1");assert(get("sdmc:/config/CardsNX/settings.json")=="saved avatars");
    // A later rename collision must restore an already replaced file.
    put("sdmc:/switch/SwitchU/first.bin","old");put(std::filesystem::path(kStagingRoot)/"switch/SwitchU/first.bin","new");put(std::filesystem::path(kStagingRoot)/"switch/SwitchU/collision.bin","new");std::filesystem::create_directories("sdmc:/switch/SwitchU/collision.bin");bool failed=false;try{installFiles({"switch/SwitchU/first.bin","switch/SwitchU/collision.bin"},false,progress,0,1);}catch(...){failed=true;}assert(failed);assert(get("sdmc:/switch/SwitchU/first.bin")=="old");assert(std::filesystem::is_directory("sdmc:/switch/SwitchU/collision.bin"));assert(get("sdmc:/config/CardsNX/settings.json")=="saved avatars");
    // Pending replacements retry after a blocked destination is released. Entries
    // owned by the original manager must survive this updater's restricted pass.
    const std::string pendingPath="sdmc:/switch/SwitchU/pending.bin";
    const std::string foreignPath="sdmc:/switch/OtherApp/other.nro";
    put(pendingPath,"old");put(pendingPath+".homenx-pending","new");
    put(pendingPath+".homenx-old/blocker","keep");
    put(kPendingList,pendingPath+"\n"+foreignPath+"\n");
    ReleaseUpdater::applyPending();assert(get(pendingPath)=="old");assert(get(pendingPath+".homenx-pending")=="new");assert(get(kPendingList)==pendingPath+"\n"+foreignPath+"\n");
    std::filesystem::remove_all(pendingPath+".homenx-old");ReleaseUpdater::applyPending();
    assert(get(pendingPath)=="new");assert(!std::filesystem::exists(pendingPath+".homenx-pending"));assert(get(kPendingList)==foreignPath+"\n");
    std::filesystem::copy_file(fixtures/"incomplete.zip",kArchivePath,std::filesystem::copy_options::overwrite_existing);failed=false;try{extractArchive(progress,0,1);}catch(...){failed=true;}assert(failed);
    ReleaseUpdater::requestCancel();std::filesystem::copy_file(fixtures/"valid.zip",kArchivePath,std::filesystem::copy_options::overwrite_existing);failed=false;try{extractArchive(progress,0,1);}catch(...){failed=true;}assert(failed);ReleaseUpdater::clearCancel();
    std::filesystem::current_path(cwd);std::filesystem::remove_all(root);std::cout<<"Updater versions, trusted release selection, path restrictions, dual installation, profile preservation, rollback, incomplete ZIP and cancellation passed. No network or Switch services used.\n";
}
