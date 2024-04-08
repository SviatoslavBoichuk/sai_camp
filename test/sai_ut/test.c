#include <stdio.h>
#include <string.h>
#include "sai.h"

const char* test_profile_get_value(
    _In_ sai_switch_profile_id_t profile_id,
    _In_ const char* variable)
{
    return 0;
}

int test_profile_get_next_value(
    _In_ sai_switch_profile_id_t profile_id,
    _Out_ const char** variable,
    _Out_ const char** value)
{
    return -1;
}

const service_method_table_t test_services = {
    test_profile_get_value,
    test_profile_get_next_value
};

int main()
{
    sai_status_t              status;
    sai_switch_api_t         *switch_api;
    sai_lag_api_t            *lag_api;
    sai_attribute_t           attrs[2];
    sai_switch_notification_t notifications;
    sai_object_id_t           port_list[64];
    sai_object_id_t           lag_oid[2];
    sai_object_id_t           lag_member_oid[4];

    status = sai_api_initialize(0, &test_services);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to initialize SAI API, status == %d\n", status);
        return status;
    }

    status = sai_api_query(SAI_API_SWITCH, (void**)&switch_api);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to query SAI API for API ID == %d, status == %d\n", SAI_API_SWITCH, status);
        return status;
    }

    status = switch_api->initialize_switch(0, "HW_ID", 0, &notifications);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to initialize switch, status == %d\n", status);
        return status;
    }

    attrs[0].id = SAI_SWITCH_ATTR_PORT_LIST;
    attrs[0].value.objlist.list = port_list;
    attrs[0].value.objlist.count = 64;
    status = switch_api->get_switch_attribute(1, attrs);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to get SAI ATTR (ATTR ID == %d), status == %d\n", SAI_SWITCH_ATTR_PORT_LIST, status);
        return status;
    }

    for (int32_t ii = 0; ii < attrs[0].value.objlist.count; ii++) {
        printf("Port #%d OID: 0x%lX\n", ii, attrs[0].value.objlist.list[ii]);
    }

    status = sai_api_query(SAI_API_LAG, (void**)&lag_api);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to query SAI API for API ID == %d, status == %d\n", SAI_API_SWITCH, status);
        return status;
    }

    status = lag_api->create_lag(&lag_oid[0], 0, NULL);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to create a LAG #1, status == %d\n", status);
        return 1;
    }

    status = lag_api->create_lag_member(&lag_member_oid[0], 0, NULL);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to create a LAG MEMBER #1, status == %d\n", status);
        return 1;
    }

    status = lag_api->create_lag_member(&lag_member_oid[1], 0, NULL);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to create a LAG MEMBER #2, status == %d\n", status);
        return 1;
    }

    status = lag_api->create_lag(&lag_oid[1], 0, NULL);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to create a LAG #2, status == %d\n", status);
        return 1;
    }

    status = lag_api->create_lag_member(&lag_member_oid[2], 0, NULL);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to create a LAG MEMBER #3, status == %d\n", status);
        return 1;
    }

    status = lag_api->create_lag_member(&lag_member_oid[3], 0, NULL);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to create a LAG MEMBER #4, status == %d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    attrs[0].value.objlist.list = port_list;
    attrs[0].value.objlist.count = 64;
    status = lag_api->get_lag_attribute(lag_oid[0], 1, attrs);
    // skip check for get

    attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    attrs[0].value.objlist.list = port_list;
    attrs[0].value.objlist.count = 64;
    status = lag_api->get_lag_attribute(lag_oid[1], 1, attrs);
     // skip check for get

    sai_object_id_t lag_id;
    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.objlist.list = &lag_id;
    attrs[0].value.objlist.count = 1;
    status = lag_api->get_lag_attribute(lag_member_oid[0], 1, attrs);
    // skip check for get

    sai_object_id_t port_id;
    attrs[0].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attrs[0].value.objlist.list = &port_id;
    attrs[0].value.objlist.count = 1;
    status = lag_api->get_lag_member_attribute(lag_member_oid[2], 1, attrs);
    // skip check for get

    status = lag_api->remove_lag_member(lag_member_oid[1]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to remove LAG MEMBER #2, status == %d\n", status);
        return status;
    }

    attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    attrs[0].value.objlist.list = port_list;
    attrs[0].value.objlist.count = 64;
    status = lag_api->get_lag_attribute(lag_oid[0], 1, attrs);
    // skip check for get    

    status = lag_api->remove_lag_member(lag_member_oid[2]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to remove LAG MEMBER #3, status == %d\n", status);
        return status;
    }

    attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    attrs[0].value.objlist.list = port_list;
    attrs[0].value.objlist.count = 64;
    status = lag_api->get_lag_attribute(lag_oid[1], 1, attrs);
    // skip check for get

    status = lag_api->remove_lag_member(lag_member_oid[0]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to remove LAG MEMBER #1, status == %d\n", status);
        return status;
    }

    status = lag_api->remove_lag_member(lag_member_oid[4]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to remove LAG MEMBER #4, status == %d\n", status);
        return status;
    }

    status = lag_api->remove_lag(lag_oid[1]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to remove LAG #2, status == %d\n", status);
        return status;
    }

    status = lag_api->remove_lag(lag_oid[0]);
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to remove LAG MEMBER #1, status == %d\n", status);
        return status;
    }
 
    switch_api->shutdown_switch(0);
    status = sai_api_uninitialize();
    if (SAI_STATUS_SUCCESS != status) {
        printf("Failed to initialize SAI API, status == %d\n", status);
        return status;
    }

    return 0;
}