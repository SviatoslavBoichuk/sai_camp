#include "sai.h"
#include "stub_sai.h"
#include "assert.h"

#define MAX_NUMBER_OF_PORTS 32
#define MAX_NUMBER_OF_LAG_MEMBERS 16
#define MAX_NUMBER_OF_LAGS 5

typedef struct _lag_member_db_entry_t {
    bool            is_used;
    sai_object_id_t port_oid;
    sai_object_id_t lag_oid;
} lag_member_db_entry_t;

typedef struct _lag_db_entry_t {
    bool            is_used;
    sai_object_id_t members_ids[MAX_NUMBER_OF_LAG_MEMBERS];
} lag_db_entry_t;

struct lag_db_t {
    lag_db_entry_t        lags[MAX_NUMBER_OF_LAGS];
    lag_member_db_entry_t members[MAX_NUMBER_OF_LAG_MEMBERS];
} lag_db;

sai_status_t get_lag_attribute(
    _In_ const sai_object_key_t *key,
    _In_ sai_attribute_value_t  *value,
    _In_ uint32_t                attr_index,
    _Inout_ vendor_cache_t      *cache,
    void                        *arg);

sai_status_t get_lag_member_attribute(
    _In_ const sai_object_key_t *key,
    _In_ sai_attribute_value_t  *value,
    _In_ uint32_t                attr_index,
    _Inout_ vendor_cache_t      *cache,
    void                        *arg);

static const sai_attribute_entry_t lag_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST, false, false, false, true,
      "List of ports in LAG", SAI_ATTR_VAL_TYPE_OBJLIST},
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t lag_vendor_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST,
      { false, false, false, true },
      { false, false, false, true },
      get_lag_attribute, (void*)SAI_LAG_ATTR_PORT_LIST,
      NULL, NULL }
};

static const sai_attribute_entry_t lag_member_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID,true, true, false, true,
      "LAG ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_LAG_MEMBER_ATTR_PORT_ID, true, true, false, true,
      "PORT ID", SAI_ATTR_VAL_TYPE_OID },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t lag_member_vendor_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID,
      { true, false, false, true },
      { true, false, false, true },
      get_lag_member_attribute, (void*) SAI_LAG_MEMBER_ATTR_LAG_ID,
      NULL, NULL },
    { SAI_LAG_MEMBER_ATTR_PORT_ID,
      { true, false, false, true },
      { true, false, false, true },
      get_lag_member_attribute, (void*) SAI_LAG_MEMBER_ATTR_PORT_ID,
      NULL, NULL }
};

sai_status_t stub_create_lag(
    _Out_ sai_object_id_t *lag_id,
    _In_ uint32_t          attr_count,
    _In_ sai_attribute_t  *attr_list)
{
    sai_status_t status;
    char         list_str[MAX_LIST_VALUE_STR_LEN];

    memset((void *)list_str, 0, MAX_LIST_VALUE_STR_LEN);

    status = check_attribs_metadata(attr_count, attr_list, lag_attribs, lag_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed attributes check\n");
        return status;
    }

    uint32_t ii = 0;
    for (; ii < MAX_NUMBER_OF_LAGS; ii++) {
        if(!lag_db.lags[ii].is_used) {
            break;
        }
    }

    if (ii == MAX_NUMBER_OF_LAGS) {
        printf("Cannot create LAG: limit is reached\n");
        return SAI_STATUS_FAILURE;
    }

    status = sai_attr_list_to_str(attr_count, attr_list, lag_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to convert attr list to string\n");
        return status;
    } else {
        printf("SAI attr list: %s\n", list_str);
    }

    lag_db.lags[ii].is_used = true;
    status = stub_create_object(SAI_OBJECT_TYPE_LAG, ii, lag_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create a LAG OID\n");
        return status;
    }

    return status;
}

bool stub_check_lag_has_members(uint32_t lag_db_id)
{
    bool     has_memers = false;
    uint32_t ii = 0;

    for (; ii < MAX_NUMBER_OF_LAG_MEMBERS; ++ii)
    {
        if (lag_db.lags[lag_db_id].members_ids[ii] != 0)
        {
            has_memers = true;
            break;
        }
    }

    return has_memers;
}

sai_status_t stub_remove_lag(_In_ sai_object_id_t  lag_id)
{
    sai_status_t status;
    uint32_t     lag_db_id;

    status = stub_object_to_type(lag_id, SAI_OBJECT_TYPE_LAG, &lag_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG DB ID.\n");
        return status;
    }

    if (stub_check_lag_has_members(lag_db_id)) {
        printf("Failed to remove LAG ID (0x%lX) - it still has LAG MEMBER(S)\n", lag_id);
        return SAI_STATUS_FAILURE;
    }

    printf("REMOVE LAG ID: (0x%lX), DB ID (%u)\n", lag_id, lag_db_id);

    lag_db.lags[lag_db_id].is_used = false;
    memset(lag_db.lags[lag_db_id].members_ids, 0, sizeof(lag_db.lags[lag_db_id].members_ids));

    return status;
}

sai_status_t stub_set_lag_attribute(
    _In_ sai_object_id_t        lag_id,
    _In_ const sai_attribute_t *attr)
{
    printf("Called function: stub_set_lag_attribute, LAG: 0x%lX\n", lag_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_attribute(
    _In_ sai_object_id_t     lag_id,
    _In_ uint32_t            attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .object_id = lag_id };

    printf("Called function: stub_get_lag_attribute, LAG: 0x%lX\n", lag_id);

    return sai_get_attributes(&key, NULL, lag_attribs, lag_vendor_attribs, attr_count, attr_list);;
}

sai_status_t get_lag_attribute(
    _In_ const sai_object_key_t *key,
    _In_ sai_attribute_value_t  *value,
    _In_ uint32_t                attr_index,
    _Inout_ vendor_cache_t      *cache,
    void                        *arg)
{
    sai_status_t          status;
    uint32_t              lag_db_index;
    uint32_t              ii;
    sai_attribute_value_t port_id;
    sai_object_id_t       port_list[MAX_NUMBER_OF_PORTS];

    status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_LAG, &lag_db_index);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Cannot get LAG DB index\n");
        return status;
    }
 
    switch((int64_t)arg) {
        case SAI_LAG_ATTR_PORT_LIST:
            ii = 0;
            for (; ii != MAX_NUMBER_OF_LAG_MEMBERS; ++ii)
            {
                const sai_object_key_t lag_member = {.object_id = lag_db.lags[lag_db_index].members_ids[ii]};
                status = get_lag_member_attribute(&lag_member, &port_id, 0, NULL, (void*)SAI_LAG_MEMBER_ATTR_PORT_ID);
                if (status == SAI_STATUS_SUCCESS)
                {
                    port_list[ii++] = port_id.oid;
                }
            }

            memcpy(value->objlist.list, port_list, sizeof(port_list));
            value->objlist.count = ii;
            break;
        default:
            printf("Got unexpected attribute ID\n");
            return SAI_STATUS_FAILURE;
    }

    return status;
}

sai_status_t stub_create_lag_member(
    _Out_ sai_object_id_t *lag_member_id,
    _In_ uint32_t          attr_count,
    _In_ sai_attribute_t  *attr_list)
{
    sai_status_t                 status;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t *lag_id, *port_id;
    uint32_t                     lag_db_index, lag_id_index, port_id_index;

    memset((void*)list_str, 0, MAX_LIST_VALUE_STR_LEN);

    status = check_attribs_metadata(attr_count, attr_list, lag_member_attribs, lag_member_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed attributes check\n");
        return status;
    }

    uint32_t ii = 0;
    for (; ii < MAX_NUMBER_OF_LAG_MEMBERS; ii++) {
        if(!lag_db.members[ii].is_used) {
            break;
        }
    }

    if (ii == MAX_NUMBER_OF_LAG_MEMBERS) {
        printf("Cannot create LAG MEMBER: limit is reached\n");
        return SAI_STATUS_FAILURE;
    }

    status = sai_attr_list_to_str(attr_count, attr_list, lag_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to convert attr list to string\n");
        return status;
    } else {
        printf("SAI attr list: %s\n", list_str);
    }

    status = find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_LAG_ID, &lag_id, &lag_id_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("LAG_ID attribute not found\n");
        return status;
    }

    status = find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_PORT_ID, &port_id, &port_id_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("LAG_ID attribute not found\n");
        return status;
    }

    lag_db.members[ii].lag_oid = lag_id->oid;
    lag_db.members[ii].port_oid = port_id->oid;
    lag_db.members[ii].is_used = true;

    status = stub_create_object(SAI_OBJECT_TYPE_LAG_MEMBER, ii, lag_member_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create a LAG MEMBER OID\n");
        return status;
    }

    status = stub_object_to_type(lag_id->oid, SAI_OBJECT_TYPE_LAG, &lag_db_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to get LAG DB index\n");
        return status;
    }

    lag_db.lags[lag_db_index].members_ids[ii] = *lag_member_id;

    return status;
}

sai_status_t stub_remove_lag_member(_In_ sai_object_id_t  lag_member_id)
{
    sai_status_t status;
    uint32_t     lag_db_id, lag_member_db_id;

    status = stub_object_to_type(lag_member_id, SAI_OBJECT_TYPE_LAG_MEMBER, &lag_member_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG MEMBER DB ID.\n");
        return status;
    }

    status = stub_object_to_type(lag_db.members[lag_member_db_id].lag_oid, SAI_OBJECT_TYPE_LAG, &lag_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG DB ID.\n");
        return status;
    }

    printf("REMOVE LAG MEMBER ID: (0x%lX), DB ID (%u)\n", lag_member_id, lag_member_db_id);

    lag_db.members[lag_member_db_id].is_used = false;
    memset(&lag_db.members[lag_member_db_id], 0, sizeof(lag_db.members[lag_member_db_id]));
    memset(&lag_db.lags[lag_db_id].members_ids[lag_member_db_id], 0, sizeof(lag_db.lags[lag_db_id].members_ids[lag_member_db_id]));

    return status;
}

sai_status_t stub_set_lag_member_attribute(
    _In_ sai_object_id_t        lag_member_id,
    _In_ const sai_attribute_t *attr)
{
    printf("Called function: stub_set_lag_member_attribute, LAG MEMBER: 0x%lX\n", lag_member_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_member_attribute(
    _In_ sai_object_id_t     lag_member_id,
    _In_ uint32_t            attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .object_id = lag_member_id };

    printf("Called function: stub_get_lag_member_attribute LAG MEMBER: 0x%lX\n", lag_member_id);

    return sai_get_attributes(&key, NULL, lag_member_attribs, lag_member_vendor_attribs, attr_count, attr_list);
}

sai_status_t get_lag_member_attribute(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg)
{
    sai_status_t status;
    uint32_t     db_index;

    assert((SAI_LAG_MEMBER_ATTR_LAG_ID == (int64_t)arg) || (SAI_LAG_MEMBER_ATTR_PORT_ID == (int64_t)arg));

    status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_LAG_MEMBER, &db_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG MEMBER DB index.\n");
        return status;
    }

    switch ((int64_t)arg) {
    case SAI_LAG_MEMBER_ATTR_LAG_ID:
        value->oid = lag_db.members[db_index].lag_oid;
        break;
    case SAI_LAG_MEMBER_ATTR_PORT_ID:
        value->oid = lag_db.members[db_index].port_oid;
        break;
    default:
        printf("Got unexpected attribute ID\n");
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

const sai_lag_api_t lag_api = {
    stub_create_lag,
    stub_remove_lag,
    stub_set_lag_attribute,
    stub_get_lag_attribute,
    stub_create_lag_member,
    stub_remove_lag_member,
    stub_set_lag_member_attribute,
    stub_get_lag_member_attribute
};
