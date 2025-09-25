#pragma once

#include "Types.h"

DYNXX_EXTERN_C_BEGIN

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

typedef address DynXXJsonNodeHandle;
typedef address DynXXJsonDecoderHandle;

/**
 * @brief Read json node type
 * @param node JSON node
 * @return `DynXXJsonNodeType`
 */
DynXXJsonNodeType dynxx_json_node_read_type(const DynXXJsonNodeHandle node);

/**
 * @brief Read json node name
 * @param node JSON node
 * @return Formatted String
 */
const char *dynxx_json_node_read_name(const DynXXJsonNodeHandle node);

/**
* @brief Read json node type
* @param node JSON node
* @return Formatted String
*/
const char *dynxx_json_node_to_str(const DynXXJsonNodeHandle node);

/**
 * @brief initialize JSON decoder
 * @param json JSON string
 * @return JSON decoder
 */
DynXXJsonDecoderHandle dynxx_json_decoder_init(const char *json);

/**
 * @brief Read JSON node wihh name
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @param k JSON node name
 * @return JSON node
 */
DynXXJsonNodeHandle dynxx_json_decoder_read_node(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node, const char *k);

/**
 * @brief Read string from the JSON node
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @return String
 */
const char *dynxx_json_decoder_read_string(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node);

/**
 * @brief Read integer number from the JOSON node
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @return Number
 */
int64_t dynxx_json_decoder_read_integer(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node);

/**
 * @brief Read float number from the JOSON node
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @return Number
 */
double dynxx_json_decoder_read_float(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node);

/**
 * @brief Read first child node of the Object/Array node
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @return First child node
 */
DynXXJsonNodeHandle dynxx_json_decoder_read_child(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node);

/**
 * @brief Read children count of the Object/Array node
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @return Children count
 */
int dynxx_json_decoder_read_children_count(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node);

/**
 * @brief Read next node
 * @param decoder JSON decoder
 * @param node JSON node, `nullptr` represents the root
 * @return Next node
 */
DynXXJsonNodeHandle dynxx_json_decoder_read_next(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node);

/**
 * @brief Release JSON decoder
 * @param decoder JSON decoder
 */
void dynxx_json_decoder_release(const DynXXJsonDecoderHandle decoder);

DYNXX_EXTERN_C_END
