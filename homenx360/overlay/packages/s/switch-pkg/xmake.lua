-- Fetch installed devkitPro libraries without sharing mutable variables across
-- concurrent package lookups. Also respect a relocated DEVKITPRO SDK.
package("switch-pkg")
    set_kind("template")
    set_description("Installed devkitPro Switch package metadata")

    on_fetch(function(package)
        local name = assert(package:data("pkgname"))
        local sdk = os.getenv("DEVKITPRO") or "/opt/devkitpro"
        import("lib.detect.find_tool")
        local pacman = find_tool("dkp-pacman") or find_tool("pacman")
        if not pacman then return end
        local listing = os.iorunv(pacman.program, {"-Ql", name})
        local includes, linkdirs, links, pcfiles = {}, {}, {}, {}
        for _, line in ipairs(listing:split('\n', {plain = true})) do
            local file = line:match("^%S+%s+(.+)$")
            if file then
                file = file:gsub("/opt/devkitpro", sdk):trim()
                if file:endswith("/include/") then table.insert(includes, file) end
                if file:endswith(".a") then
                    table.insert(linkdirs, path.directory(file))
                    table.insert(links, path.basename(file):gsub("^lib", ""))
                end
                if file:endswith(".pc") then table.insert(pcfiles, file) end
            end
        end
        import("package.manager.pkgconfig.find_package", {alias = "findpc"})
        for _, file in ipairs(pcfiles) do
            local found = findpc(path.basename(file), {configdirs = path.directory(file)})
            if found then
                table.join2(includes, found.includedirs or {})
                table.join2(linkdirs, found.linkdirs or {})
                table.join2(links, found.links or {})
            end
        end
        return {includedirs = table.unique(includes), linkdirs = table.unique(linkdirs),
                links = table.reverse_unique(links)}
    end)

    on_install(function(package)
        raise("Install devkitPro package '%s' in your SDK before building", package:data("pkgname"))
    end)
package_end()
