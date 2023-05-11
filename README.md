# WdmFilterDriverTemplate
implement 2 drivers, one is standar wdm driver, another is its filter driver, which can be attach on standard driver. this can be observed by devtree.exe utility on Windows

how to use
1. build these 2 dirvers with VS2019.
2. build consoletest.exe with VS2019
3. install standard WDM driver first
4. devcon.exe install MyStandardDriver root\MyStandardNode
5. install filter WDM driver.
6. devcon.exe install MyFilterDriver.inf root\MyFilterNode
7. test via consoletest.exe, you will see fiter (enter) --> standard (enter) --> standard (leave) --> filter (leave).
