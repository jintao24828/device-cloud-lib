/**
 * @file
 * @brief unit testing for IoT library (alarm source file)
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "test_support.h"

#include "api/public/iot.h"
#include "api/shared/iot_types.h"
#include "iot_build.h"

static void test_iot_alarm_register_empty( void **state )
{
	size_t i;
	iot_t lib;
	char name[] = "alarm";
	iot_alarm_t *result = NULL;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 0u;
	result = iot_alarm_register( &lib, name );
	assert_non_null( result );
	assert_int_equal( lib.alarm_count, 1u );
	assert_ptr_equal( result, lib.alarm_ptr[0] );
}

static void test_iot_alarm_register_full( void **state )
{
	size_t i;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN + 1u];
	char *t_names;
	iot_alarm_t *result;
	iot_alarm_t *stack_alarm;

	t_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_ALARM_MAX );
	assert_non_null( t_names );

	stack_alarm = (iot_alarm_t *)test_calloc( IOT_ALARM_MAX - IOT_ALARM_STACK_MAX,
	                                                  sizeof( iot_alarm_t ) );
	assert_non_null( stack_alarm );
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_MAX; i++ )
	{
		if ( i < IOT_ALARM_STACK_MAX )
			lib.alarm_ptr[i] = &lib.alarm[i];
		else
			lib.alarm_ptr[i] = &stack_alarm[i - IOT_ALARM_STACK_MAX];
		lib.alarm_ptr[i]->name = &t_names[( IOT_NAME_MAX_LEN + 1u ) * i];
		snprintf( lib.alarm_ptr[i]->name, IOT_NAME_MAX_LEN, "alarm %03lu", i );
	}
	snprintf( name, IOT_NAME_MAX_LEN, "alarm %03d.5", IOT_ALARM_MAX / 2u );
	lib.alarm_count = IOT_ALARM_MAX;
	result = iot_alarm_register( &lib, name );
	assert_null( result );
	assert_int_equal( lib.alarm_count, IOT_ALARM_MAX );
	test_free( t_names );
	test_free( stack_alarm );
}

static void test_iot_alarm_register_stack_full( void **state )
{
	size_t i;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN + 1u];
	char *t_names;
	iot_alarm_t *result;

	t_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_ALARM_STACK_MAX );
	assert_non_null( t_names );
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
	{
		lib.alarm_ptr[i] = &lib.alarm[i];
		lib.alarm_ptr[i]->name = &t_names[( IOT_NAME_MAX_LEN + 1u ) * i];
		snprintf( lib.alarm_ptr[i]->name, IOT_NAME_MAX_LEN, "alarm %03lu", i );
	}
	snprintf( name, IOT_NAME_MAX_LEN, "alarm %03d.5", IOT_ALARM_STACK_MAX / 2u );
	lib.alarm_count = IOT_ALARM_STACK_MAX;
	result = iot_alarm_register( &lib, name );
	if ( IOT_ALARM_MAX > IOT_ALARM_STACK_MAX )
	{
		assert_non_null( result );
		assert_int_equal( lib.alarm_count, IOT_ALARM_STACK_MAX + 1u );
#ifndef IOT_STACK_ONLY
		assert_int_equal( result->is_in_heap, 1 );
#endif
		os_free( result );
	}
	else
	{
		assert_null( result );
		assert_int_equal( lib.alarm_count, IOT_ALARM_MAX );
	}
	test_free( t_names );
}

static void test_iot_alarm_register_null_lib( void **state )
{
	char name[] = "alarm";
	iot_alarm_t *result = NULL;

	result = iot_alarm_register( NULL, name );
	assert_null( result );
}

static void test_iot_alarm_register_null_name( void **state )
{
	size_t i;
	iot_t lib;
	iot_alarm_t *result = NULL;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 0u;
	result = iot_alarm_register( &lib, NULL );
	assert_null( result );
	assert_int_equal( lib.alarm_count, 0u );
}

static void test_iot_alarm_register_valid( void **state )
{
	size_t i;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN];
	iot_alarm_t *result = NULL;
	char *t_names;

	t_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_ALARM_STACK_MAX );
	assert_non_null( t_names );

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
	{
		lib.alarm_ptr[i] = &lib.alarm[i];
		lib.alarm_ptr[i]->name = &t_names[( IOT_NAME_MAX_LEN + 1u ) * i];
		snprintf( lib.alarm_ptr[i]->name, IOT_NAME_MAX_LEN, "alarm %03lu", i );
	}
	lib.alarm_count = IOT_ALARM_STACK_MAX - 1u;
	snprintf( name, IOT_NAME_MAX_LEN, "alarm %03d.5", IOT_ALARM_STACK_MAX / 2u );
	result = iot_alarm_register( &lib, name );
	assert_non_null( result );
	assert_int_equal( lib.alarm_count, IOT_ALARM_STACK_MAX );
	assert_ptr_equal( result, lib.alarm_ptr[IOT_ALARM_STACK_MAX / 2u + 1] );
	test_free( t_names );
}

static void test_iot_alarm_deregister_null_alarm( void **state )
{
	iot_status_t result;
	result = iot_alarm_deregister( NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_alarm_deregister_null_lib( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 2u;
	alarm = lib.alarm_ptr[1];
	alarm->lib = NULL;
	result = iot_alarm_deregister( alarm );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
	assert_int_equal( lib.alarm_count, 2u );
}

static void test_iot_alarm_deregister_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 2u;
	alarm = lib.alarm_ptr[1];
	alarm->lib = &lib;
	result = iot_alarm_deregister( alarm );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.alarm_count, 1u );
}

static void test_iot_alarm_publish_null_lib( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 1u;
	alarm = lib.alarm_ptr[0];
	alarm->lib = NULL;
	result = iot_alarm_publish( alarm, 1, "" );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
}

static void test_iot_alarm_publish_null_alarm( void **state )
{
	iot_status_t result;

	result = iot_alarm_publish( NULL, 1, "" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_alarm_publish_no_memory( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 1u;
	alarm = lib.alarm_ptr[0];
	alarm->lib = &lib;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_alarm_publish( alarm, 1, "alarm message" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_alarm_publish_null_message( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 1u;
	alarm = lib.alarm_ptr[0];
	alarm->lib = &lib;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_alarm_publish( alarm, 1, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_alarm_publish_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 1u;
	alarm = lib.alarm_ptr[0];
	alarm->lib = &lib;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_NO_MEMORY );
	result = iot_alarm_publish( alarm, 1, "alarm message" );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_alarm_register_empty ),
		cmocka_unit_test( test_iot_alarm_register_full ),
		cmocka_unit_test( test_iot_alarm_register_stack_full ),
		cmocka_unit_test( test_iot_alarm_register_null_lib ),
		cmocka_unit_test( test_iot_alarm_register_null_name ),
		cmocka_unit_test( test_iot_alarm_register_valid ),
		cmocka_unit_test( test_iot_alarm_deregister_null_alarm ),
		cmocka_unit_test( test_iot_alarm_deregister_null_lib ),
		cmocka_unit_test( test_iot_alarm_deregister_valid ),
		cmocka_unit_test( test_iot_alarm_publish_null_lib ),
		cmocka_unit_test( test_iot_alarm_publish_null_alarm ),
		cmocka_unit_test( test_iot_alarm_publish_no_memory ),
		cmocka_unit_test( test_iot_alarm_publish_null_message ),
		cmocka_unit_test( test_iot_alarm_publish_valid )
	};
	MOCK_SYSTEM_ENABLED = 1;
	result = cmocka_run_group_tests( tests, NULL, NULL );
	MOCK_SYSTEM_ENABLED = 0;
	return result;
}