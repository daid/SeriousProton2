#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

//If we require static objects to be initialized before any user created object, we can apply this attribute to the static object.
// Example are the keybindings, we want to initilize our list of key bindings before any of the keybindings itself is initialized.
#define SP2_INIT_EARLY __attribute__((init_priority(101)))

#define SP2_DEPRECATED(msg) __attribute__((deprecated(msg)))

#endif//ATTRIBUTES_H
