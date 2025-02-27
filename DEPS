use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'dawn_git': 'https://dawn.googlesource.com',
  'github_git': 'https://github.com',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'dawn_standalone': True,
  'dawn_node': False, # Also fetches dependencies required for building NodeJS bindings.
  'dawn_cmake_version': 'version:3.13.5',
  'dawn_cmake_win32_sha1': 'b106d66bcdc8a71ea2cdf5446091327bfdb1bcd7',
  'dawn_gn_version': 'git_revision:fc295f3ac7ca4fe7acc6cb5fb052d22909ef3a8f',
  'dawn_go_version': 'version:1.16',
  'webnn_enable_onednn': False, # For webnn CI
}

deps = {
  # Dependencies required for tests.
  'webnn/node/third_party/webnn-polyfill': {
    'url': '{github_git}/webmachinelearning/webnn-polyfill.git@9ef11ab490be1275df801a43a7b81977b6966560'
  },
  'webnn/node/third_party/webnn-polyfill/test-data': {
    'url': '{github_git}/webmachinelearning/test-data.git@b6f1565fefc103705a6ff580067eae7bb9d3b351'
  },
  'webnn/node/third_party/webnn-samples': {
    'url': '{github_git}/webmachinelearning/webnn-samples.git@d358fe623be5236736ebc2d896cfbba2af4fd348'
  },
  'webnn/node/third_party/webnn-samples/test-data': {
    'url': '{github_git}/webmachinelearning/test-data.git@b6f1565fefc103705a6ff580067eae7bb9d3b351',
  },
  'webnn/third_party/stb': {
    'url': '{github_git}/nothings/stb@b42009b3b9d4ca35bc703f5310eedc74f584be58'
  },

  # Dependencies required for backends.
  'webnn/third_party/DirectML': {
    'url': '{github_git}/microsoft/DirectML.git@af53ef743559079bd2ebb1aa83d9f98e87d774e1',
    'condition': 'checkout_win',
  },
  # GPGMM support for fast DML allocation and residency management.
  'webnn/third_party/gpgmm': {
    'url': '{github_git}/intel/gpgmm.git@7c81fae56e30c60030cb0a2c53310723e5c085d6',
    'condition': 'checkout_win',
  },
  'webnn/third_party/oneDNN': {
    'url': '{github_git}/oneapi-src/oneDNN.git@4a129541fd4e67e6897072186ea2817a3154eddd',
    'condition': 'webnn_enable_onednn',
  },

  # Dependencies required to use GN/Clang in standalone
  'build': {
    'url': '{chromium_git}/chromium/src/build@0ff4b3d4eeb6d480c716b432a9a93a58c42150d5',
    'condition': 'dawn_standalone',
  },
  'buildtools': {
    'url': '{chromium_git}/chromium/src/buildtools@9c143ace7560797fed136da85e22ea4834e6b147',
    'condition': 'dawn_standalone',
  },
  'buildtools/clang_format/script': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@99803d74e35962f63a775f29477882afd4d57d94',
    'condition': 'dawn_standalone',
  },

  'buildtools/linux64': {
    'packages': [{
      'package': 'gn/gn/linux-amd64',
      'version': Var('dawn_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'dawn_standalone and host_os == "linux"',
  },
  'buildtools/mac': {
    'packages': [{
      'package': 'gn/gn/mac-${{arch}}',
      'version': Var('dawn_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'dawn_standalone and host_os == "mac"',
  },
  'buildtools/win': {
    'packages': [{
      'package': 'gn/gn/windows-amd64',
      'version': Var('dawn_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'dawn_standalone and host_os == "win"',
  },

  'buildtools/third_party/libc++/trunk': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxx.git@8fa87946779682841e21e2da977eccfb6cb3bded',
    'condition': 'dawn_standalone',
  },

  'buildtools/third_party/libc++abi/trunk': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxxabi.git@f4328ad7c0d8242d36cb5bea530925f9fea34248',
    'condition': 'dawn_standalone',
  },

  'tools/clang': {
    'url': '{chromium_git}/chromium/src/tools/clang@03ff857f12277f511e0a30aca44b80e8aaebafd7',
    'condition': 'dawn_standalone',
  },
  'tools/clang/dsymutil': {
    'packages': [{
      'package': 'chromium/llvm-build-tools/dsymutil',
      'version': 'M56jPzDv1620Rnm__jTMYS62Zi8rxHVq7yw0qeBFEgkC',
    }],
    'condition': 'dawn_standalone and (checkout_mac or checkout_ios)',
    'dep_type': 'cipd',
  },

  # Testing, GTest and GMock
  'testing': {
    'url': '{chromium_git}/chromium/src/testing@3e2640a325dc34ec3d9cb2802b8da874aecaf52d',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest': {
    'url': '{chromium_git}/external/github.com/google/googletest@2828773179fa425ee406df61890a150577178ea2',
    'condition': 'dawn_standalone',
  },

  # Jinja2 and MarkupSafe for the code generator
  'third_party/jinja2': {
    'url': '{chromium_git}/chromium/src/third_party/jinja2@ee69aa00ee8536f61db6a451f3858745cf587de6',
    'condition': 'dawn_standalone',
  },
  'third_party/markupsafe': {
    'url': '{chromium_git}/chromium/src/third_party/markupsafe@0944e71f4b2cb9a871bcbe353f95e889b64a611a',
    'condition': 'dawn_standalone',
  },

  # WGSL support
  'third_party/tint': {
    'url': '{dawn_git}/tint@cc4d97b6e31962c0d5645e8ed7a4bdfbfc4d7339',
  },

  # GLFW for tests and samples
  'third_party/glfw': {
    'url': '{chromium_git}/external/github.com/glfw/glfw@94773111300fee0453844a4c9407af7e880b4df8',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan_memory_allocator': {
    'url': '{chromium_git}/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@5e49f57a6e71a026a54eb42e366de09a4142d24e',
    'condition': 'dawn_standalone',
  },

  'third_party/angle': {
    'url': '{chromium_git}/angle/angle@bc9d2d7de9d4f789218de0c47c9026c8c122f16d',
    'condition': 'dawn_standalone',
  },

  'third_party/swiftshader': {
    'url': '{swiftshader_git}/SwiftShader@f354daff5a2b882fc182c8377c826aa99b30f989',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-deps': {
    'url': '{chromium_git}/vulkan-deps@fbeca8f4ea6a6280982422028e991c2821385383',
    'condition': 'dawn_standalone',
  },

  'third_party/zlib': {
    'url': '{chromium_git}/chromium/src/third_party/zlib@c29ee8c9c3824ca013479bf8115035527967fe02',
    'condition': 'dawn_standalone',
  },

  'third_party/abseil-cpp': {
    'url': '{chromium_git}/chromium/src/third_party/abseil-cpp@789af048b388657987c59d4da406859034fe310f',
    'condition': 'dawn_standalone',
  },

  # Dependencies required to build Dawn NodeJS bindings
  'third_party/node-api-headers': {
    'url': '{github_git}/nodejs/node-api-headers.git@d68505e4055ecb630e14c26c32e5c2c65e179bba',
    'condition': 'dawn_node',
  },
  'third_party/node-addon-api': {
    'url': '{github_git}/nodejs/node-addon-api.git@4a3de56c3e4ed0031635a2f642b27efeeed00add',
    'condition': 'dawn_node',
  },
  'third_party/gpuweb': {
    'url': '{github_git}/gpuweb/gpuweb.git@67edc187f5305a72456663c34d51153601b79f3b',
    'condition': 'dawn_node',
  },

  'tools/golang': {
    'condition': 'dawn_node',
    'packages': [{
      'package': 'infra/3pp/tools/go/${{platform}}',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
  },

  'tools/cmake': {
    'condition': 'dawn_node and (host_os == "mac" or host_os == "linux")',
    'packages': [{
      'package': 'infra/3pp/tools/cmake/${{platform}}',
      'version': Var('dawn_cmake_version'),
    }],
    'dep_type': 'cipd',
  },
}

hooks = [
  # Pull the compilers and system libraries for hermetic builds
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_linux and (checkout_x86 or checkout_x64)',
    'action': ['python', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_linux and checkout_x64',
    'action': ['python', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    # Update the Mac toolchain if possible, this makes builders use "hermetic XCode" which is
    # is more consistent (only changes when rolling build/) and is cached.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_mac',
    'action': ['python', 'build/mac_toolchain.py'],
  },
  {
    # Update the Windows toolchain if necessary. Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win',
    'action': ['python', 'build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'tools/clang/scripts/update.py'],
    'condition': 'dawn_standalone',
  },
  {
    # Pull rc binaries using checked-in hashes.
    'name': 'rc_win',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/win/rc.exe.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "mac"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "linux"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/linux64/clang-format.sha1',
    ],
  },
  # Update build/util/LASTCHANGE.
  {
    'name': 'lastchange',
    'pattern': '.',
    'condition': 'dawn_standalone',
    'action': ['python', 'build/util/lastchange.py',
               '-o', 'build/util/LASTCHANGE'],
  },
  # TODO(https://crbug.com/1180257): Use CIPD for CMake on Windows.
  {
    'name': 'cmake_win32',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-tools',
                Var('dawn_cmake_win32_sha1'),
                '-o', 'tools/cmake-win32.zip'
    ],
  },
  {
    'name': 'cmake_win32_extract',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "win"',
    'action': [ 'python',
                'scripts/extract.py',
                'tools/cmake-win32.zip',
                'tools/cmake-win32/',
    ],
  },
  {
    # Download the DirectML NuGet package.
    'name': 'download_dml_unpkg',
    'pattern': '.',
    'condition': 'checkout_win',
    'action': ['python3', 'webnn/src/webnn_native/dml/deps/script/download_dml.py'],
  }
]

recursedeps = [
  'third_party/vulkan-deps',
]
