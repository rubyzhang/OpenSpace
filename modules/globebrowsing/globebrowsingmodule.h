/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2019                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __OPENSPACE_MODULE_GLOBEBROWSING___GLOBEBROWSING_MODULE___H__
#define __OPENSPACE_MODULE_GLOBEBROWSING___GLOBEBROWSING_MODULE___H__

#include <openspace/properties/stringproperty.h>
#include <openspace/properties/scalar/boolproperty.h>
#include <openspace/properties/scalar/uintproperty.h>
#include <openspace/util/openspacemodule.h>

#include <ghoul/glm.h>
#include <memory>
#include <future>

namespace openspace::globebrowsing {
    class RenderableGlobe;
    struct TileIndex;
    struct Geodetic2;
    struct Geodetic3;

    namespace cache { class MemoryAwareTileCache; }
} // namespace openspace::globebrowsing

namespace openspace {

class Camera;

class GlobeBrowsingModule : public OpenSpaceModule {
public:
    constexpr static const char* Name = "GlobeBrowsing";

    GlobeBrowsingModule();

    void goToChunk(const globebrowsing::RenderableGlobe& globe, int x, int y, int level);
    void goToGeo(const globebrowsing::RenderableGlobe& globe,
        double latitude, double longitude);

    void goToGeo(const globebrowsing::RenderableGlobe& globe,
        double latitude, double longitude, double altitude);

    glm::vec3 cartesianCoordinatesFromGeo(const globebrowsing::RenderableGlobe& globe,
        double latitude, double longitude, double altitude);

    globebrowsing::cache::MemoryAwareTileCache* tileCache();
    scripting::LuaLibrary luaLibrary() const override;
    std::vector<documentation::Documentation> documentations() const override;

    const globebrowsing::RenderableGlobe* castFocusNodeRenderableToGlobe();

    struct Layer {
        std::string name;
        std::string url;
    };
    using Capabilities = std::vector<Layer>;

    // Stores the mapping between globe to names
    struct UrlInfo {
        std::string name;
        std::string url;
    };

    // Registers then user-usable name
    void loadWMSCapabilities(std::string name, std::string globe, std::string url);
    Capabilities capabilities(const std::string& name);

    std::vector<UrlInfo> urlInfo(const std::string& globe) const;
    bool hasUrlInfo(const std::string& globe) const;

    void removeWMSServer(const std::string& name);

    bool isWMSCachingEnabled() const;
    bool isInOfflineMode() const;
    std::string wmsCacheLocation() const;
    uint64_t wmsCacheSize() const; // bytes

#ifdef OPENSPACE_MODULE_GLOBEBROWSING_INSTRUMENTATION
    void addFrameInfo(globebrowsing::RenderableGlobe* globe, uint32_t nTilesRenderedLocal,
        uint32_t nTilesRenderedGlobal, uint32_t nTilesUploaded);
#endif // OPENSPACE_MODULE_GLOBEBROWSING_INSTRUMENTATION

protected:
    void internalInitialize(const ghoul::Dictionary&) override;

private:
    void goToChunk(const globebrowsing::RenderableGlobe& globe,
        const globebrowsing::TileIndex& ti, glm::vec2 uv, bool doResetCameraDirection);

    void goToGeodetic2(const globebrowsing::RenderableGlobe& globe,
        globebrowsing::Geodetic2 geo2, bool doResetCameraDirection);

    void goToGeodetic3(const globebrowsing::RenderableGlobe& globe,
        globebrowsing::Geodetic3 geo3, bool doResetCameraDirection);

    glm::dquat lookDownCameraRotation(const globebrowsing::RenderableGlobe& globe,
        glm::dvec3 cameraPositionModelSpace, globebrowsing::Geodetic2 geo2);

    /**
     \return a comma separated list of layer group names.
     */
    static std::string layerGroupNamesList();

    /**
     \return a comma separated list of layer type names.
     */
    static std::string layerTypeNamesList();


    properties::BoolProperty _wmsCacheEnabled;
    properties::BoolProperty _offlineMode;
    properties::StringProperty _wmsCacheLocation;
    properties::UIntProperty _wmsCacheSizeMB;
    properties::UIntProperty _tileCacheSizeMB;

    std::unique_ptr<globebrowsing::cache::MemoryAwareTileCache> _tileCache;

    // name -> capabilities
    std::map<std::string, std::future<Capabilities>> _inFlightCapabilitiesMap;
    // name -> capabilities
    std::map<std::string, Capabilities> _capabilitiesMap;

    std::multimap<std::string, UrlInfo> _urlList;

#ifdef OPENSPACE_MODULE_GLOBEBROWSING_INSTRUMENTATION
    struct FrameInfo {
        uint64_t iFrame = 0;
        uint32_t nTilesRenderedLocal = 0;
        uint32_t nTilesRenderedGlobal = 0;
        uint32_t nTilesUploaded = 0;
    };

    struct {
        std::unordered_map<
            globebrowsing::RenderableGlobe*,
            std::vector<FrameInfo>
        > frames;

        uint64_t lastSavedFrame = 0;
        const uint16_t saveEveryNthFrame = 2048;
    } _frameInfo;
    properties::BoolProperty _saveInstrumentation;
#endif // OPENSPACE_MODULE_GLOBEBROWSING_INSTRUMENTATION
};

} // namespace openspace

#endif // __OPENSPACE_MODULE_GLOBEBROWSING___GLOBEBROWSING_MODULE___H__
