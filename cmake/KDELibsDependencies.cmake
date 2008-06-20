# this file was generated during the kdelibs build process
set(KDE_VERSION_MAJOR 4)
set(KDE_VERSION_MINOR 0u)
set(KDE_VERSION_RELEASE 1)
set(KDE_VERSION "4.0.1")
set(KDE_VERSION_STRING "4.0.1 (KDE 4.0.1)")

if (NOT KDE4_INSTALL_DIR)
   set(KDE4_INSTALL_DIR         "/usr/kde/4.0")
endif (NOT KDE4_INSTALL_DIR)

set(KDE4_LIB_INSTALL_DIR     "/usr/kde/4.0/lib")
set(KDE4_LIBEXEC_INSTALL_DIR "/usr/kde/4.0/lib/kde4/libexec")
set(KDE4_INCLUDE_INSTALL_DIR "/usr/kde/4.0/include")
set(KDE4_BIN_INSTALL_DIR     "/usr/kde/4.0/bin")
set(KDE4_SBIN_INSTALL_DIR    "/usr/kde/4.0/sbin")
set(KDE4_DATA_INSTALL_DIR    "/usr/kde/4.0/share/apps")
set(KDE4_HTML_INSTALL_DIR    "/usr/kde/4.0/share/doc/HTML")
set(KDE4_CONFIG_INSTALL_DIR  "/usr/kde/4.0/share/config")
set(KDE4_ICON_INSTALL_DIR    "/usr/kde/4.0/share/icons")
set(KDE4_KCFG_INSTALL_DIR    "/usr/kde/4.0/share/config.kcfg")
set(KDE4_LOCALE_INSTALL_DIR  "/usr/kde/4.0/share/locale")
set(KDE4_MIME_INSTALL_DIR    "/usr/kde/4.0/share/mimelnk")
set(KDE4_SOUND_INSTALL_DIR   "/usr/kde/4.0/share/sounds")
set(KDE4_TEMPLATES_INSTALL_DIR "/usr/kde/4.0/share/templates")
set(KDE4_WALLPAPER_INSTALL_DIR "/usr/kde/4.0/share/wallpapers")
set(KDE4_KCONF_UPDATE_INSTALL_DIR "/usr/kde/4.0/share/apps/kconf_update")
set(KDE4_AUTOSTART_INSTALL_DIR "/usr/kde/4.0/share/autostart")
set(KDE4_XDG_APPS_INSTALL_DIR        "/usr/kde/4.0/share/applications/kde4")
set(KDE4_XDG_DIRECTORY_INSTALL_DIR   "/usr/kde/4.0/share/desktop-directories")
set(KDE4_SYSCONF_INSTALL_DIR "/usr/kde/4.0/etc")
set(KDE4_MAN_INSTALL_DIR     "/usr/kde/4.0/share/man")
set(KDE4_INFO_INSTALL_DIR    "/usr/kde/4.0/share/info")
set(KDE4_DBUS_INTERFACES_DIR "/usr/kde/4.0/share/dbus-1/interfaces")
set(KDE4_DBUS_SERVICES_DIR   "/usr/kde/4.0/share/dbus-1/services")
set(KDE4_SERVICES_INSTALL_DIR "/usr/kde/4.0/share/kde4/services")
set(KDE4_SERVICETYPES_INSTALL_DIR "/usr/kde/4.0/share/kde4/servicetypes")

SET(kdecore_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtNetwork.so;/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtXml.so;/lib/libz.so;/lib/libbz2.so;resolv;")
SET(kdefakes_LIB_DEPENDS "")
SET(ktranscript_LIB_DEPENDS "kjs;/usr/lib/qt4/libQtCore.so;-lpthread;")
SET(klibloadertestmodule_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtTest.so;")
SET(klibloadertestmodule4_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtTest.so;")
SET(kded_globalaccel_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;")
SET(kdeui_LIB_DEPENDS "/usr/lib/qt4/libQtSvg.so;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;-lSM;/usr/lib/libICE.so;/usr/lib/libX11.so;/usr/lib/libXext.so;/usr/lib/libXft.so;/usr/lib/libXau.so;/usr/lib/libXdmcp.so;/usr/lib/libXpm.so;/usr/lib/qt4/libQtGui.so;/usr/lib/qt4/libQtXml.so;/usr/lib/libXtst.so;/usr/lib/libXcursor.so;/usr/lib/libXfixes.so;")
SET(kpty_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;util;utempter;")
SET(kdesu_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kpty;")
SET(kjs_LIB_DEPENDS "-lpthread;m;/usr/lib/libpcre.so;/usr/lib/libpcreposix.so;")
SET(kjsembed_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtUiTools.a;/usr/lib/qt4/libQtGui.so;/usr/lib/qt4/libQtSvg.so;/usr/lib/qt4/libQtXml.so;kjs;")
SET(kwalletbackend_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;")
SET(kio_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;/lib/libz.so;/usr/lib/libstreamanalyzer.so;/usr/lib/libstreams.so;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;solid;/usr/lib/libfam.so;/lib/libacl.so;/lib/libattr.so;/usr/lib/libXrender.so;")
SET(kded_proxyscout_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kio;kjs;resolv;")
SET(kded_kssld_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kio;")
SET(kded_kwalletd_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kwalletbackend;")
SET(kntlm_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;")
SET(dummy_LIB_DEPENDS "/usr/lib/libstreamanalyzer.so;")
SET(phonon_LIB_DEPENDS "/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtGui.so;")
SET(phonon_fake_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtGui.so;phonon;phononexperimental;")
SET(phononexperimental_LIB_DEPENDS "/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtGui.so;phonon;")
SET(kde_LIB_DEPENDS "/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtGui.so;phonon;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;")
SET(kaudiodevicelist_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;solid;/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtGui.so;phonon;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;/usr/lib/libasound.so;")
SET(kcm_phonon_LIB_DEPENDS "/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtGui.so;phonon;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kutils;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;solid;kaudiodevicelist;")
SET(solid_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtXml.so;/usr/lib/qt4/libQtGui.so;")
SET(solid_static_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtXml.so;/usr/lib/qt4/libQtGui.so;")
SET(kdeinit_kbuildsycoca4_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;")
SET(kdeinit_kded4_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;")
SET(kde3support_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kpty;/usr/lib/qt4/libQtXml.so;/usr/lib/qt4/libQt3Support.so;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kfile;")
SET(kunittest_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;")
SET(kfile_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;/lib/libz.so;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;solid;")
SET(kfilemodule_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kde3support;kfile;")
SET(kdeinit_kconf_update_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;")
SET(kio_ghelp_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kio;/usr/lib/libxml2.so;/usr/lib/libxslt.so;")
SET(kio_help_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kio;/usr/lib/libxml2.so;/usr/lib/libxslt.so;")
SET(kio_file_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kio;")
SET(kdeinit_kio_http_cache_cleaner_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;")
SET(kio_http_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kntlm;")
SET(kded_kcookiejar_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;/usr/lib/qt4/libQt3Support.so;")
SET(kio_ftp_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kio;")
SET(kio_metainfo_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kio;")
SET(knewstuff2_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;/usr/lib/qt4/libQtNetwork.so;")
SET(kparts_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;")
SET(notepadpart_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;")
SET(spellcheckplugin_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;")
SET(kutils_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;")
SET(kdeinit_klauncher_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;")
SET(threadweaver_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;")
SET(kspell_aspell_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/libaspell.so;")
SET(kspell_enchant_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/libenchant.so;")
SET(khtml_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;ktexteditor;kjs;/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtGui.so;phonon;/usr/lib/libjpeg.so;/usr/lib/libgif.so;/usr/lib/libpng.so;/lib/libz.so;")
SET(khtmladaptorpart_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;kjs;")
SET(khtmlimagepart_LIB_DEPENDS "khtml;")
SET(khtmlpart_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;khtml;")
SET(kmultipart_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;")
SET(kjavaappletviewer_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;/usr/lib/qt4/libQt3Support.so;")
SET(ktexteditor_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtDBus.so;kparts;")
SET(kmediaplayer_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;")
SET(kfileaudiopreview_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtGui.so;phonon;")
SET(kimproxy_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;")
SET(kdewidgets_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kde3support;")
SET(katepart_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kutils;ktexteditor;kjs;kde3support;")
SET(ktexteditor_insertfile_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;ktexteditor;")
SET(ktexteditor_kdatatool_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;ktexteditor;")
SET(ktexteditor_docwordcompletion_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;ktexteditor;")
SET(knotifyconfig_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;/usr/lib/qt4/libQtDBus.so;/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtGui.so;phonon;")
SET(kimg_dds_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtGui.so;")
SET(kimg_eps_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtGui.so;")
SET(kimg_ico_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtGui.so;")
SET(kimg_jp2_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtGui.so;/usr/lib/libjasper.so;/usr/lib/libjpeg.so;")
SET(kimg_pcx_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtGui.so;")
SET(kimg_psd_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtGui.so;")
SET(kimg_rgb_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtGui.so;")
SET(kimg_tga_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtGui.so;")
SET(kimg_xcf_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtGui.so;")
SET(kimg_xview_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtGui.so;")
SET(kdnssd_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;/usr/lib/libdns_sd.so;")
SET(krosscore_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;")
SET(krossui_LIB_DEPENDS "krosscore;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;")
SET(krossmoduleforms_LIB_DEPENDS "/usr/lib/qt4/libQtUiTools.a;/usr/lib/qt4/libQtDesigner.so;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kde3support;kfile;krosscore;krossui;")
SET(krosskjs_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;krosscore;kjs;kjsembed;")
SET(kcm_crypto_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kde3support;/usr/lib/libssl.so;")
SET(kcertpart_LIB_DEPENDS "/usr/lib/qt4/libQtCore.so;-lpthread;kdecore;kdeui;kio;kparts;/usr/lib/qt4/libQt3Support.so;")
SET(nepomuk_LIB_DEPENDS "/usr/lib/libsoprano.so;/usr/lib/libsopranoclient.so;/usr/lib/qt4/libQtCore.so;-lpthread;/usr/lib/qt4/libQtGui.so;/usr/lib/qt4/libQtDBus.so;kdeui;")