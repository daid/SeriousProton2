#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

//If we require static objects to be initialized before any user created object, we can apply this attribute to the static object.
// Example are the keybindings, we want to initilize our list of key bindings before any of the keybindings itself is initialized.
#define SP2_INIT_EARLY __attribute__((init_priority(101)))
//If we define something that is a static global initializer being added to a list, add this attribute to ensure it
// is not removed by some optimizing step.
#define SP2_STATIC_INIT __attribute__((used))

#define SP2_DEPRECATED(msg) __attribute__((deprecated(msg)))

#if defined(__clang__)
#define SP2_PACKED __attribute__((__packed__))
#else
#define SP2_PACKED __attribute__((__packed__, __gcc_struct__))
#endif

#define SP2_NO_RETURN __attribute__((noreturn))

#endif//ATTRIBUTES_H
