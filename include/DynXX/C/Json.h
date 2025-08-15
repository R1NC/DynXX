#ifndef DYNXX_INCLUDE_JSON_H_
#define DYNXX_INCLUDE_JSON_H_

#include "Types.h"

EXTERN_C_BEGIN

enum DynXXJsonNodeType {
    DynXXJsonNodeTypeUnknown,
    DynXXJsonNodeTypeObject,
    DynXXJsonNodeTypeArray,
    DynXXJsonNodeTypeString,
    DynXXJsonNodeTypeInt32,
    DynXXJsonNodeTypeInt64,
    DynXXJsonNodeTypeFloat,
    DynXXJsonNodeTypeBoolean,
    DynXXJsonNodeTypeNull
};

/**
 * @brief Read json node type
 * @param cjson JSON node
 * @return `DynXXJsonNodeType`
 */
int dynxx_json_read_type(void *const cjson);

/**
* @brief Read json node type
* @param cjson JSON node
* @return Formatted String
*/
const char *dynxx_json_to_str(void *const cjson);

/**
 * @brief initialize JSON decoder
 * @param json JSON string
 * @return JSON decoder
 */
void *dynxx_json_decoder_init(const char *json);

/**
 * @brief Read JSON node wihh name
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @param k JSON node name
 * @return JSON node
 */
void *dynxx_json_decoder_read_node(void *const decoder, void *const node, const char *k);

/**
 * @brief Read string from the JSON node
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @return String
 */
const char *dynxx_json_decoder_read_string(void *const decoder, void *const node);

/**
 * @brief Read number from the JOSON node
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @return Number
 */
double dynxx_json_decoder_read_number(void *const decoder, void *const node);

/**
 * @brief Read first child node of the Object/Array node
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @return First child node
 */
void *dynxx_json_decoder_read_child(void *const decoder, void *const node);

/**
 * @brief Read next node
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @return Next node
 */
void *dynxx_json_decoder_read_next(void *const decoder, void *const node);

/**
 * @brief Release JSON decoder
 * @param decoder JSON decoder
 */
void dynxx_json_decoder_release(void *const decoder);

EXTERN_C_END

#endif // DYNXX_INCLUDE_JSON_H_
