# ADB
PRODUCT_SYSTEM_DEFAULT_PROPERTIES += \
    ro.adb.nonblocking_ffs=false \
    service.adb.root=1
    
# DRM
PRODUCT_PROPERTY_OVERRIDES += \
    drm.service.enabled=true \
    ro.com.widevine.cachesize=16777216        
    
# Facelock
PRODUCT_PROPERTY_OVERRIDES += \
    ro.facelock.black_timeout=700 \
    ro.facelock.det_timeout=2500 \
    ro.facelock.rec_timeout=3500 \
    ro.facelock.est_max_time=500

# Fdsan
PRODUCT_PROPERTY_OVERRIDES += \
    debug.fdsan=warn_once
    
# Google Assistant
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opa.eligible_device=true    
    
# Healthd
PRODUCT_PROPERTY_OVERRIDES += \
    ro.enable_boot_charger_mode=1

# LMK
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

# Locale
PRODUCT_SYSTEM_PROPERTY_BLACKLIST += \
    ro.product.locale
    
# Mobile Data Provision
PRODUCT_PROPERTY_OVERRIDES += \
    ro.com.android.prov_mobiledata=false

# OEM Unlock
PRODUCT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=1 \
    ro.frp.pst=/dev/block/platform/700b0600.sdhci/by-name/PST

# OMX
PRODUCT_PROPERTY_OVERRIDES += \
    debug.stagefright.ccodec=0 \
    debug.stagefright.omx_default_rank=0 \
    debug.stagefright.omx_default_rank.sw-audio=1

# Recovery
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.recovery_update=false \
    persist.sys.binary_xml=false

