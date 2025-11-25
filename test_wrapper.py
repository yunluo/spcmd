#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
测试SPCMD Python包装器
"""

import spcmd_wrapper

def test_random_ids():
    """测试随机ID生成"""
    print("测试UUID v4生成:")
    uuid4_ids = spcmd_wrapper.generate_uuid4(3)
    for uuid in uuid4_ids:
        print(f"  {uuid}")
    
    print("\n测试UUID v7生成:")
    uuid7_ids = spcmd_wrapper.generate_uuid7(2)
    for uuid in uuid7_ids:
        print(f"  {uuid}")
    
    print("\n测试雪花ID生成:")
    snowflake_ids = spcmd_wrapper.generate_snowflake_ids(3)
    for sid in snowflake_ids:
        print(f"  {sid}")
    
    print("\n测试随机数生成:")
    random_numbers = spcmd_wrapper.generate_random_numbers(5)
    for num in random_numbers:
        print(f"  {num}")

if __name__ == "__main__":
    test_random_ids()