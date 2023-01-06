// swift-tools-version:5.5
import PackageDescription

let package = Package(
  name: "gambit",
  products: [
    .library(
      name: "gambit",
      type: .dynamic,
      targets: ["gambit"]),
  ],
  dependencies: [
  ],
  targets: [
    .target(
      name: "gambit",
      dependencies: [],
      path: "src",
      publicHeadersPath: "include",
      cxxSettings: [.headerSearchPath("")]
    ),
  ],
  cxxLanguageStandard: .cxx11
)
