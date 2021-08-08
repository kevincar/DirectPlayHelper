
# Headers
Get-ChildItem -Recurse ".\include" | where { $_.extension -eq ".hpp" } | % { cpplint --filter="-build/c++11,-legal/copyright" $(Resolve-Path -Relative $_.FullName) }

# Source
Get-ChildItem -Recurse ".\src" | where { $_.extension -eq ".cpp" } | % { cpplint --filter="-build/c++11,-legal/copyright" $(Resolve-Path -Relative $_.FullName) }

# Test
Get-ChildItem -Recurse ".\tests" | where { $_.extension -eq ".hpp" } | % { cpplint --filter="-build/c++11,-legal/copyright" $(Resolve-Path -Relative $_.FullName) }
Get-ChildItem -Recurse ".\tests" | where { $_.extension -eq ".cpp" } | % { cpplint --filter="-build/c++11,-legal/copyright" $(Resolve-Path -Relative $_.FullName) }