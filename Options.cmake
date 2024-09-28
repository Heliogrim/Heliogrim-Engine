include_guard(GLOBAL)

# ------------------------------
# Heliogrim Global Build Options
# ------------------------------

make_option(VAR "HELIOGRIM_USE_MAIN_THREAD_INJECTION" NAME "USE_MAIN_THREAD_INJECTION" DEFAULT OFF)
make_option(VAR "HELIOGRIM_PREVENT_MAIN_AUTO_SLEEP" NAME "PREVENT_MAIN_AUTO_SLEEP" DEFAULT OFF)
make_option(VAR "HELIOGRIM_FULL_STATIC_LINKING" NAME "FULL_STATIC_LINKING" CALLBACK support_opt_linux)

# ------------------------------

make_option(VAR "HELIOGRIM_PLATFORM_WAYLAND" NAME "PLATFORM_WAYLAND" CALLBACK support_opt_wayland)
make_option(VAR "HELIOGRIM_PLATFORM_X11" NAME "PLATFORM_X11" CALLBACK support_opt_x11)

# ------------------------------
# Heliogrim Vendor Options
# ------------------------------

make_option(VAR "HELIOGRIM_ASSETS_FORCE_HTTPS" NAME "ASSETS_FORCE_HTTPS" DEFAULT OFF)

# ------------------------------
