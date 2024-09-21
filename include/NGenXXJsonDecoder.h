#ifndef NGENXX_JSON_DECODER_H_
#define NGENXX_JSON_DECODER_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief initialize JSON decoder
     * @param json JSON string
     * @return JSON decoder
     */
    void *ngenxx_json_decoder_init(const char *json);

    /**
     * @brief Check if the node is an array
     * @param decoder JSON decoder
     * @param node JSON node, `NULL` represents the root
     * @return true if the node is an array
     */
    bool ngenxx_json_decoder_is_array(void *decoder, void *node);

    /**
     * @brief Check if the node is an object
     * @param decoder JSON decoder
     * @param node JSON node, `NULL` represents the root
     * @return true if the node is an array
     */
    bool ngenxx_json_decoder_is_object(void *decoder, void *node);

    /**
     * @brief Read JSON node wihh name
     * @param decoder JSON decoder
     * @param node JSON node, `NULL` represents the root
     * @param k JSON node name
     * @return JSON node
     */
    void *ngenxx_json_decoder_read_node(void *decoder, void *node, const char *k);

    /**
     * @brief Read string from the JSON node
     * @param decoder JSON decoder
     * @param node JSON node, `NULL` represents the root
     * @return String
     */
    const char *ngenxx_json_decoder_read_string(void *decoder, void *node);

    /**
     * @brief Read number from the JOSON node
     * @param decoder JSON decoder
     * @param node JSON node, `NULL` represents the root
     * @return Number
     */
    double ngenxx_json_decoder_read_number(void *decoder, void *node);

    /**
     * @brief Read first child node of the Object/Array node
     * @param decoder JSON decoder
     * @param node JSON node, `NULL` represents the root
     * @return First child node
     */
    void *ngenxx_json_decoder_read_child(void *decoder, void *node);

    /**
     * @brief Read next node
     * @param decoder JSON decoder
     * @param node JSON node, `NULL` represents the root
     * @return Next node
     */
    void *ngenxx_json_decoder_read_next(void *decoder, void *node);

    /**
     * @brief Release JSON decoder
     * @param decoder JSON decoder
     */
    void ngenxx_json_decoder_release(void *decoder);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_JSON_DECODER_H_