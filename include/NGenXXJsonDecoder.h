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
     * @param obj JSON node, `NULL` represents the root
     * @return true if the node is an array
     */
    bool ngenxx_json_decoder_is_array(void *decoder, void *obj);

    /**
     * @brief Read JSON node
     * @param decoder JSON decoder
     * @param obj JSON node, `NULL` represents the root
     * @param k JSON node name
     * @return JSON node
     */
    void *ngenxx_json_decoder_read_item(void *decoder, void *obj, const char *k);

    /**
     * @brief Read string from the JSON node
     * @param decoder JSON decoder
     * @param obj JSON node, `NULL` represents the root
     * @return String
     */
    const char *ngenxx_json_decoder_read_string(void *decoder, void *obj);

    /**
     * @brief Read number from the JOSON node
     * @param decoder JSON decoder
     * @param obj JSON node, `NULL` represents the root
     * @return Number
     */
    double ngenxx_json_decoder_read_number(void *decoder, void *obj);

    /**
     * @brief Read first node in the array
     * @param decoder JSON decoder
     * @param obj JSON node, `NULL` represents the root
     * @return First node of the array
     */
    void *ngenxx_json_decoder_read_array(void *decoder, void *obj);

    /**
     * @brief Read next node in the array
     * @param decoder JSON decoder
     * @param obj JSON node, `NULL` represents the root
     * @return Next node in the array
     */
    void *ngenxx_json_decoder_read_array_next(void *decoder, void *obj);

    /**
     * @brief Release JSON decoder
     * @param decoder JSON decoder
     */
    void ngenxx_json_decoder_release(void *decoder);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_JSON_DECODER_H_