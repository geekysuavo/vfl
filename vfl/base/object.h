
/* ensure once-only inclusion. */
#ifndef __VFL_OBJECT_H__
#define __VFL_OBJECT_H__

/* include c library headers. */
#include <stdlib.h>
#include <string.h>

/* OBJECT_TYPE(): macro function for casting object structure pointers
 * to their associated base type structures.
 */
#define OBJECT_TYPE(s) ((object_type_t*) (s)->type)

/* OBJECT_IS_NIL(): macro function to check if an object is empty.
 */
#define OBJECT_IS_NIL(obj) \
  (OBJECT_TYPE(obj) == vfl_object_nil)

/* object_t: defined type for the base object structure. */
typedef struct object object_t;

/* object_method_fn(): execute an object method.
 *
 * arguments:
 *  @obj: object structure pointer to call with.
 *  @args: mapping holding method arguments.
 *  @out: pointer to the output value.
 *
 * returns:
 *  output from the method, or null on failure.
 */
typedef object_t* (*object_method_fn) (object_t *obj, const object_t *args);

/* object_init_fn(): initialize the contents of an object.
 *
 * arguments:
 *  @obj: object structure pointer to initialize.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*object_init_fn) (object_t *obj);

/* object_copy_fn(): copy allocated contents between objects.
 *
 * arguments:
 *  @obj: source object structure pointer.
 *  @objdup: destination object structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*object_copy_fn) (const object_t *obj,
                               object_t *objdup);

/* object_free_fn(): free any allocated contents of an object.
 *
 * arguments:
 *  @obj: object structure pointer to free.
 */
typedef void (*object_free_fn) (object_t *obj);

/* object_test_fn(): assert the contents of an object.
 *
 * arguments:
 *  @obj: object structure pointer to test.
 *
 * returns:
 *  integer indicating a true (1) or false (0) assertion.
 */
typedef int (*object_test_fn) (const object_t *obj);

/* object_comp_fn(): perform a binary comparison of two objects.
 *
 * arguments:
 *  @a, @b: input object structure pointers.
 *
 * returns:
 *  integer indicating the comparison result:
 *   -1,  if  a < b
 *    0,  if  a = b
 *    1,  if  a > b
 *  322,  if  error
 */
#define OBJECT_CMP_ERR  322
typedef int (*object_comp_fn) (const object_t *a, const object_t *b);

/* object_binary_fn(): perform a binary operation on two objects.
 *
 * arguments:
 *  @a, @b: input object structure pointers.
 *
 * returns:
 *  output object structure pointer.
 */
typedef object_t* (*object_binary_fn) (const object_t *a,
                                       const object_t *b);

/* object_getprop_fn(): get the value of an object's property.
 *
 * arguments:
 *  @obj: object structure pointer.
 *
 * returns:
 *  object property value.
 */
typedef object_t* (*object_getprop_fn) (const object_t *obj);

/* object_getelem_fn(): get the value of an object's element.
 *
 * arguments:
 *  @obj: object structure pointer.
 *  @idx: element index.
 *
 * returns:
 *  object element value.
 */
typedef object_t* (*object_getelem_fn) (const object_t *obj,
                                        const object_t *idx);

/* object_setprop_fn(): set the value of an object's property.
 *
 * arguments:
 *  @obj: object structure pointer.
 *  @prop: property value.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*object_setprop_fn) (object_t *obj, object_t *prop);

/* object_setelem_fn(): set the value of an object's element.
 *
 * arguments:
 *  @obj: object structure pointer.
 *  @idx: element index.
 *  @elem: element value.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*object_setelem_fn) (object_t *obj, const object_t *idx,
                                  object_t *elem);

/* object_property_t: structure containing information about
 * accessible object properties.
 */
typedef struct {
  /* property attributes:
   *  @name: property name string.
   *  @get: getter function pointer.
   *  @set: setter function pointer.
   */
  const char *name;
  object_getprop_fn get;
  object_setprop_fn set;
}
object_property_t;

/* object_method_t: structure containing information about
 * a callable method.
 */
typedef struct {
  /* method attributes:
   *  @name: method string name.
   *  @fn: method function pointer.
   */
  const char *name;
  object_method_fn fn;
}
object_method_t;

/* object_type_t: structure for holding type-specific information.
 */
typedef struct {
  /* basic object parameters:
   *  @name: string name of the allocated object.
   *  @size: number of bytes allocated to the structure pointer.
   */
  const char *name;
  long size;

  /* core object functions:
   *  @init: initialization hook.
   *  @copy: deep copying hook.
   *  @free: deallocation hook.
   *  @test: assertion hook.
   *  @cmp: comparison hook.
   */
  object_init_fn init;
  object_copy_fn copy;
  object_free_fn free;
  object_test_fn test;
  object_comp_fn cmp;

  /* arithmetic object functions:
   *  @add: addition hook.
   *  @sub: subtraction hook.
   *  @mul: multiplication hook.
   *  @div: division hook.
   *  @pow: exponentiation hook.
   */
  object_binary_fn add;
  object_binary_fn sub;
  object_binary_fn mul;
  object_binary_fn div;
  object_binary_fn pow;

  /* element access functions:
   *  @get: element getter hook.
   *  @set: element setter hook.
   */
  object_getelem_fn get;
  object_setelem_fn set;

  /* property and method tables:
   *  @props: object property table.
   *  @methods: object method table.
   */
  object_property_t *props;
  object_method_t *methods;
}
object_type_t;

/* OBJECT_BASE: base set of struct members that begin all objects.
 */
#define OBJECT_BASE \
  object_type_t *type; /* object type information.  */ \
  unsigned int refs;   /* instance reference count. */

/* struct object: structure for holding a vfl object.
 */
struct object {
  /* base structure members. */
  OBJECT_BASE;

  /* object members are placed here. */
};

/* vfl_nil: externally globally available 'empty object'.
 */
extern const object_t *vfl_nil;

/* VFL_RETURN_NIL: macro for returning an empty object from methods.
 */
#define VFL_RETURN_NIL \
  return (object_t*) vfl_nil;

/* function declarations (base/object.c): */

object_t *obj_alloc (const object_type_t *type);

object_t *obj_copy (const object_t *obj);

void obj_release (object_t *obj);

#define obj_retain(obj) \
  if (obj) { ((object_t*) (obj))->refs++; }

#define obj_collect(obj) \
  if (obj && ((object_t*) (obj))->refs == 0) { obj_free((object_t*) obj); }

void obj_free (object_t *obj);

int obj_test (const object_t *obj);

object_t *obj_add (const object_t *a, const object_t *b);

object_t *obj_sub (const object_t *a, const object_t *b);

object_t *obj_mul (const object_t *a, const object_t *b);

object_t *obj_div (const object_t *a, const object_t *b);

object_t *obj_pow (const object_t *a, const object_t *b);

object_t *obj_eq (const object_t *a, const object_t *b);

object_t *obj_ne (const object_t *a, const object_t *b);

object_t *obj_lt (const object_t *a, const object_t *b);

object_t *obj_gt (const object_t *a, const object_t *b);

object_t *obj_le (const object_t *a, const object_t *b);

object_t *obj_ge (const object_t *a, const object_t *b);

object_t *obj_getprop (const object_t *obj, const char *name);

object_t *obj_getelem (const object_t *obj, const object_t *idx);

int obj_setprop (object_t *obj, const char *name, object_t *val);

int obj_setelem (object_t *obj, const object_t *idx,
                 object_t *val);

object_t *obj_method (object_t *obj, const char *name, object_t *args);

/* available object types: */

extern const object_type_t *vfl_object_nil;

#endif /* !__VFL_OBJECT_H__ */

