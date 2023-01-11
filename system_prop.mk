PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.direct.interface=p2p-dev-wlan0 \
    ro.hwui.texture_cache_size=86 \
    ro.hwui.layer_cache_size=56 \
    ro.hwui.r_buffer_cache_size=8 \
    ro.hwui.path_cache_size=40 \
    ro.hwui.gradient_cache_size=1 \
    ro.hwui.drop_shadow_cache_size=6 \
    ro.hwui.texture_cache_flushrate=0.4 \
    ro.hwui.text_small_cache_width=1024 \
    ro.hwui.text_small_cache_height=1024 \
    ro.hwui.text_large_cache_width=2048 \
    ro.hwui.text_large_cache_height=1024 \
    ro.hwui.disable_scissor_opt=true \
    ro.recents.grid=true

# mobile data provision prop
PRODUCT_PROPERTY_OVERRIDES += \
    ro.com.android.prov_mobiledata=false

# facelock props
PRODUCT_PROPERTY_OVERRIDES += \
    ro.facelock.black_timeout=700 \
    ro.facelock.det_timeout=2500 \
    ro.facelock.rec_timeout=3500 \
    ro.facelock.est_max_time=500

PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.media_vol_steps=25

# Set fdsan to the warn_once severity level
PRODUCT_PROPERTY_OVERRIDES += \
    debug.fdsan=warn_once

# OEM Unlock reporting
PRODUCT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=1

# OpenGL ES
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=196610

# Update the recovery image only if the option is enabled
# under Developer options
# by default, do not update the recovery image
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.recovery_update=false

PRODUCT_PROPERTY_OVERRIDES += \
    ro.audio.monitorRotation=true \
    ro.frp.pst=/dev/block/platform/700b0600.sdhci/by-name/PST

PRODUCT_PROPERTY_OVERRIDES += \
    ro.bt.bdaddr_path=/sys/devices/700b0200.sdhci/mmc_host/mmc0/mmc0:0001/mmc0:0001:2/net/wlan0/address

# DRM Mappings
PRODUCT_PROPERTY_OVERRIDES += \
    camera.flash_off=0 \
    drm.service.enabled=true \
    ro.com.widevine.cachesize=16777216

# P2P0 Concurrency
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.direct.non-concurrent=true

# Google Assistant
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opa.eligible_device=true

# UI
PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.enable_hwc_vds=1 \
    debug.sf.hw=1 \
    debug.sf.latch_unsignaled=1 \
    debug.egl.hw=1 \
    debug.gralloc.enable_fb_ubwc=1 \
    dalvik.vm.dex2oat64.enabled=true \
    dev.pm.dyn_samplingrate=1 \
    persist.demo.hdmirotationlock=false \
    debug.sf.recomputecrop=0 \
    sys.use_fifo_ui=1 \
    debug.sf.enable_gl_backpressure=1 \
    debug.renderengine.backend=threaded

# Allows healthd to boot directly from charger mode rather than initiating a reboot.
PRODUCT_PROPERTY_OVERRIDES += \
    ro.enable_boot_charger_mode=1

PRODUCT_PROPERTY_OVERRIDES += \
    ro.hardware.vulkan=tegra

PRODUCT_PROPERTY_OVERRIDES += \
    debug.stagefright.ccodec=0

PRODUCT_PROPERTY_OVERRIDES += \
    ro.lmk.low=1001 \
    ro.lmk.medium=800 \
    ro.lmk.critical=0 \
    ro.lmk.critical_upgrade=true \
    ro.lmk.upgrade_pressure=60 \
    ro.lmk.downgrade_pressure=60 \
    ro.lmk.kill_heaviest_task=true \
    ro.lmk.kill_timeout_ms=100 \
    ro.lmk.use_minfree_levels=true \
    ro.lmk.use_psi=false

PRODUCT_PROPERTY_OVERRIDES += \
    ro.bionic.ld.warning=0

# System
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.binary_xml=false

# ConfigStore HAL has been deprecated in presence of these props
PRODUCT_PROPERTY_OVERRIDES += \
    ro.surface_flinger.max_frame_buffer_acquired_buffers=3 \
    ro.surface_flinger.present_time_offset_from_vsync_ns=0 \
    ro.surface_flinger.vsync_event_phase_offset_ns=7500000 \
    ro.surface_flinger.vsync_sf_event_phase_offset_ns=5000000

# set default USB and ADB configuration
PRODUCT_SYSTEM_DEFAULT_PROPERTIES += \
    persist.sys.usb.config=adb \
    ro.adb.secure=0 \
    service.adb.root=1

# Charger
PRODUCT_PRODUCT_PROPERTIES += \
    ro.charger.enable_suspend=true

# The default locale should be determined from VPD, not from build.prop.
PRODUCT_SYSTEM_PROPERTY_BLACKLIST += \
    ro.product.locale

