#LoadPlugin("I:\RIXE\tcasFilter\trunk\src\alpha\tcasFilter\Release\tcasFilter.dll")  # 路径请自行设置
LoadPlugin("I:\TheTecoProject\tcxCore project\tcxCore\branches\bin\VSFilter.dll")

BlankClip(length = 2400, width = 848, height = 480, fps = 23.976, pixel_type = "RGB24", color=$000000)
textsub("test.ass")
