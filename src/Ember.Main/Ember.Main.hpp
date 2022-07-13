#pragma once

/**
 * Ember main entry callback for framework usage
 *
 * @attention This callback is not meant for blocking calls
 * @attention This will be invoked by main thread, which is NOT a valid fiber
 *
 * @author Julius
 * @date 20.10.2021
 */
extern void ember_main_entry();
