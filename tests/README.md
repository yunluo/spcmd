# SPCMD 测试目录

此目录用于存储所有自动化测试的生成文件和日志。

## 文件说明

- `test_result.log` - 完整的测试执行日志
- `test_shot.jpg` - 截屏测试生成的JPG图像
- `test_shot.bmp` - 截屏测试生成的BMP图像
- `test_shot.png` - 截屏测试生成的PNG图像
- `test_shot_thumb.jpg` - 压缩质量测试生成的JPG图像
- `test_config.ini` - 配置文件测试生成的INI文件

## 测试执行

运行测试脚本:
```bash
.\test.bat
```

所有测试生成的文件都将保存在此目录中，便于整理和清理。

## .gitignore 说明

此目录的 `.gitignore` 配置忽略所有测试生成的文件，只提交README和.gitignore本身。

