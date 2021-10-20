#pragma once

#ifndef WAIT_USER_EXIT
/**
 * Blocking callback to keep framework alive
 */
extern void ember_block_main();
#endif

/**
 * Ember main entry callback for framework usage
 *
 * @attention This callback is not meant for blocking calls
 *
 * @author Julius
 * @date 20.10.2021
 */
extern void ember_main_entry();
