/****************************************************************************
  FileName     [ zxIO.cpp ]
  PackageName  [ zx ]
  Synopsis     [ Define class ZXGraph Reader/Writer functions ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>

#include "./zxFileParser.hpp"
#include "./zxGraph.hpp"
#include "util/logger.hpp"
#include "util/tmpFiles.hpp"

using namespace std;

extern dvlab::utils::Logger logger;

/**
 * @brief Read a ZXGraph
 *
 * @param filename
 * @param keepID if true, keep the IDs as written in file; if false, rearrange the vertex IDs
 * @return true if correctly constructed the graph
 * @return false
 */
bool ZXGraph::readZX(std::filesystem::path const& filepath, bool keepID) {
    // REVIEW - should we guard the case of no file extension?
    if (filepath.has_extension()) {
        if (filepath.extension() != ".zx" && filepath.extension() != ".bzx") {
            fmt::println("unsupported file extension \"{}\"!!", filepath.extension().string());
            return false;
        }
    }

    ZXFileParser parser;
    return parser.parse(filepath.string()) && buildGraphFromParserStorage(parser.getStorage());
}

/**
 * @brief Write a ZXGraph
 *
 * @param filename
 * @param complete
 * @return true if correctly write a graph into .zx
 * @return false
 */
bool ZXGraph::writeZX(const string& filename, bool complete) const {
    ofstream ZXFile;
    ZXFile.open(filename);
    if (!ZXFile.is_open()) {
        logger.error("Cannot open the file \"{}\"!!", filename);
        return false;
    }
    auto writeNeighbors = [&ZXFile, complete](ZXVertex* v) {
        for (const auto& [nb, etype] : v->getNeighbors()) {
            if ((complete) || (nb->getId() >= v->getId())) {
                ZXFile << " ";
                switch (etype) {
                    case EdgeType::SIMPLE:
                        ZXFile << "S";
                        break;
                    case EdgeType::HADAMARD:
                    default:
                        ZXFile << "H";
                        break;
                }
                ZXFile << nb->getId();
            }
        }
        return true;
    };
    ZXFile << "// Input \n";
    for (auto& v : _inputs) {
        ZXFile << "I" << v->getId() << " (" << v->getQubit() << "," << floor(v->getCol()) << ")";
        if (!writeNeighbors(v)) return false;
        ZXFile << "\n";
    }

    ZXFile << "// Output \n";

    for (auto& v : _outputs) {
        ZXFile << "O" << v->getId() << " (" << v->getQubit() << "," << floor(v->getCol()) << ")";
        if (!writeNeighbors(v)) return false;
        ZXFile << "\n";
    }

    ZXFile << "// Non-boundary \n";
    for (ZXVertex* const& v : _vertices) {
        if (v->isBoundary()) continue;

        if (v->isZ())
            ZXFile << "Z";
        else if (v->isX())
            ZXFile << "X";
        else
            ZXFile << "H";
        ZXFile << v->getId();

        ZXFile << " (" << v->getQubit() << "," << floor(v->getCol()) << ")";  // NOTE - always output coordinate now
        if (!writeNeighbors(v)) return false;

        if (v->getPhase() != (v->isHBox() ? Phase(1) : Phase(0))) ZXFile << " " << v->getPhase().getAsciiString();
        ZXFile << "\n";
    }
    return true;
}

/**
 * @brief Build graph from parser storage
 *
 * @param storage
 * @param keepID
 * @return true
 * @return false
 */
bool ZXGraph::buildGraphFromParserStorage(const ZXParserDetail::StorageType& storage, bool keepID) {
    unordered_map<size_t, ZXVertex*> id2Vertex;

    for (auto& [id, info] : storage) {
        ZXVertex* v = std::invoke(
            // clang++ does not support structured binding capture by reference with OpenMP
            [&id = id, &info = info, this]() {
                if (info.type == 'I')
                    return addInput(info.qubit, info.column);
                if (info.type == 'O')
                    return addOutput(info.qubit, info.column);
                VertexType vtype;
                if (info.type == 'Z')
                    vtype = VertexType::Z;
                else if (info.type == 'X')
                    vtype = VertexType::X;
                else
                    vtype = VertexType::H_BOX;
                return addVertex(info.qubit, vtype, info.phase, info.column);
            });

        if (keepID) v->setId(id);
        id2Vertex[id] = v;
    }

    for (auto& [vid, info] : storage) {
        for (auto& [type, nbid] : info.neighbors) {
            if (!id2Vertex.contains(nbid)) {
                logger.error("failed to build the graph: cannot find vertex with ID {}!!", nbid);
                return false;
            }

            if (vid < nbid) {
                addEdge(id2Vertex[vid], id2Vertex[nbid], (type == 'S') ? EdgeType::SIMPLE : EdgeType::HADAMARD);
            }
        }
    }
    return true;
}

/**
 * @brief Generate tikz file
 *
 * @param filename
 * @return true if the filename is valid
 * @return false if not
 */
bool ZXGraph::writeTikz(string const& filename) const {
    fstream tikzFile{filename, ios::out};
    if (!tikzFile.is_open()) {
        logger.error("Cannot open the file \"{}\"!!", filename);
        return false;
    }

    return writeTikz(tikzFile);
}

/**
 * @brief write tikz file to the fstream `tikzFile`
 *
 * @param tikzFile
 * @return true if the filename is valid
 * @return false if not
 */
bool ZXGraph::writeTikz(std::ostream& tikzFile) const {
    constexpr string_view defineColors =
        "\\definecolor{zx_red}{RGB}{253, 160, 162}\n"
        "\\definecolor{zx_green}{RGB}{206, 254, 206}\n"
        "\\definecolor{hedgeColor}{RGB}{40, 160, 240}\n"
        "\\definecolor{phaseColor}{RGB}{14, 39, 100}\n";

    constexpr string_view tikzStyle =
        "[\n"
        "font = \\sffamily,\n"
        "\t yscale=-1,\n"
        "\t boun/.style={circle, text=yellow!60, font=\\sffamily, draw=black!100, fill=black!60, thick, text width=3mm, align=center, inner sep=0pt},\n"
        "\t hbox/.style={regular polygon, regular polygon sides=4, font=\\sffamily, draw=yellow!40!black!100, fill=yellow!40, text width=2.5mm, align=center, inner sep=0pt},\n"
        "\t zspi/.style={circle, font=\\sffamily, draw=green!60!black!100, fill=zx_green, text width=5mm, align=center, inner sep=0pt},\n"
        "\t xspi/.style={circle, font=\\sffamily, draw=red!60!black!100, fill=zx_red, text width=5mm, align=center, inner sep=0pt},\n"
        "\t hedg/.style={draw=hedgeColor, thick},\n"
        "\t sedg/.style={draw=black, thick},\n"
        "];\n";

    static unordered_map<VertexType, string> const vt2s = {
        {VertexType::BOUNDARY, "boun"},
        {VertexType::Z, "zspi"},
        {VertexType::X, "xspi"},
        {VertexType::H_BOX, "hbox"}};

    static unordered_map<EdgeType, string> const et2s = {
        {EdgeType::HADAMARD, "hedg"},
        {EdgeType::SIMPLE, "sedg"},
    };

    constexpr string_view fontSize = "\\tiny";

    size_t max = 0;

    for (auto& v : _outputs) {
        if (max < v->getCol())
            max = v->getCol();
    }
    for (auto& v : _inputs) {
        if (max < v->getCol())
            max = v->getCol();
    }
    double scale = (double)25 / (double)static_cast<int>(max);
    scale = (scale > 3.0) ? 3.0 : scale;
    tikzFile << defineColors;
    tikzFile << "\\scalebox{" << to_string(scale) << "}{";
    tikzFile << "\\begin{tikzpicture}" << tikzStyle;
    tikzFile << "    % Vertices\n";

    auto writePhase = [&tikzFile, &fontSize](ZXVertex* v) {
        if (v->getPhase() == Phase(0) && v->getType() != VertexType::H_BOX)
            return true;
        if (v->getPhase() == Phase(1) && v->getType() == VertexType::H_BOX)
            return true;
        string labelStyle = "[label distance=-2]90:{\\color{phaseColor}";
        tikzFile << ",label={ " << labelStyle << fontSize << " $";
        int numerator = v->getPhase().numerator();
        int denominator = v->getPhase().denominator();

        if (denominator != 1) {
            tikzFile << "\\frac{";
        }
        if (numerator != 1) {
            tikzFile << "\\mathsf{" << to_string(numerator) << "}";
        }
        tikzFile << "\\pi";
        if (denominator != 1) {
            tikzFile << "}{ \\mathsf{" << to_string(denominator) << "}}";
        }
        tikzFile << "$ }}";
        return true;
    };

    // NOTE - Sample: \node[zspi] (88888)  at (0,1) {{\tiny 88888}};
    for (auto& v : _vertices) {
        tikzFile << "    \\node[" << vt2s.at(v->getType());
        writePhase(v);
        tikzFile << "]";
        tikzFile << "(" << to_string(v->getId()) << ")  at (" << to_string(v->getCol()) << "," << to_string(v->getQubit()) << ") ";
        tikzFile << "{{" << fontSize << " " << to_string(v->getId()) << "}};\n";
    }
    // NOTE - Sample: \draw[hedg] (1234) -- (123);
    tikzFile << "    % Edges\n";

    for (auto& v : _vertices) {
        for (auto& [n, e] : v->getNeighbors()) {
            if (n->getId() > v->getId()) {
                if (n->getCol() == v->getCol() && n->getQubit() == v->getQubit()) {
                    logger.warning("{} and {} are connected but they have same coordinates.", v->getId(), n->getId());
                    tikzFile << "    % \\draw[" << et2s.at(e) << "] (" << to_string(v->getId()) << ") -- (" << to_string(n->getId()) << ");\n";
                } else
                    tikzFile << "    \\draw[" << et2s.at(e) << "] (" << to_string(v->getId()) << ") -- (" << to_string(n->getId()) << ");\n";
            }
        }
    }

    tikzFile << "\\end{tikzpicture}}\n";
    return true;
}

/**
 * @brief Generate pdf file
 *
 * @param filename
 * @param toPDF if true, compile it to .pdf
 * @return true
 * @return false
 */
bool ZXGraph::writePdf(string const& filename) const {
    namespace fs = std::filesystem;
    namespace dv = dvlab::utils;
    fs::path filepath{filename};

    if (filepath.extension() == "") {
        logger.error("no file extension!!");
        return false;
    }

    if (filepath.extension() != ".pdf") {
        logger.error("unsupported file extension \"{}\"!!", filepath.extension().string());
        return false;
    }

    filepath.replace_extension(".tex");
    if (filepath.parent_path().empty()) {
        filepath = "./" + filepath.string();
    }

    std::error_code ec;
    fs::create_directory(filepath.parent_path(), ec);
    if (ec) {
        logger.error("failed to create the directory");
        logger.error("{}", ec.message());
        return false;
    }

    dv::TmpDir tmpDir;

    auto tempTexPath = tmpDir.path() / filepath.filename();

    fstream texFile{tempTexPath, ios::out};
    if (!texFile.is_open()) {
        logger.error("Cannot open the file \"{}\"!!", filepath.string());
        return false;
    }

    if (!writeTex(texFile)) return false;

    texFile.close();
    // NOTE - Linux cmd: pdflatex -halt-on-error -output-directory <path/to/dir> <path/to/tex>
    string cmd = "pdflatex -halt-on-error -output-directory " + tempTexPath.parent_path().string() + " " + tempTexPath.string() + " >/dev/null 2>&1 ";
    if (system(cmd.c_str()) == -1) {
        logger.error("failed to generate PDF");
        return false;
    }

    filepath.replace_extension(".pdf");

    if (fs::exists(filepath))
        fs::remove(filepath);

    // NOTE - copy instead of rename to avoid cross device link error
    fs::copy(tempTexPath.replace_extension(".pdf"), filepath);

    return true;
}

/**
 * @brief Generate pdf file
 *
 * @param filename
 * @param toPDF if true, compile it to .pdf
 * @return true
 * @return false
 */
bool ZXGraph::writeTex(string const& filename) const {
    namespace fs = std::filesystem;
    fs::path filepath{filename};

    if (filepath.extension() == "") {
        logger.error("no file extension!!");
        return false;
    }

    if (filepath.extension() != ".tex") {
        logger.error("unsupported file extension \"{}\"!!", filepath.extension().string());
        return false;
    }

    if (!filepath.parent_path().empty()) {
        std::error_code ec;
        fs::create_directory(filepath.parent_path(), ec);
        if (ec) {
            logger.error("failed to create the directory");
            logger.error("{}", ec.message());
            return false;
        }
    }

    fstream texFile{filepath, ios::out};
    if (!texFile.is_open()) {
        logger.error("Cannot open the file \"{}\"!!", filepath.string());
        return false;
    }

    return writeTex(texFile);
}

/**
 * @brief Generate tex file
 *
 * @param filename
 * @return true if the filename is valid
 * @return false if not
 */
bool ZXGraph::writeTex(ostream& texFile) const {
    string includes =
        "\\documentclass[a4paper,landscape]{article}\n"
        "\\usepackage[english]{babel}\n"
        "\\usepackage[top=2cm,bottom=2cm,left=1cm,right=1cm,marginparwidth=1.75cm]{geometry}"
        "\\usepackage{amsmath}\n"
        "\\usepackage{tikz}\n"
        "\\usetikzlibrary{shapes}\n"
        "\\usetikzlibrary{plotmarks}\n"
        "\\usepackage[colorlinks=true, allcolors=blue]{hyperref}\n"
        "\\usetikzlibrary{positioning}\n"
        "\\usetikzlibrary{shapes.geometric}\n";
    texFile << includes;
    texFile << "\\begin{document}\n";
    if (!writeTikz(texFile)) {
        logger.error("Failed to write tikz");
        return false;
    }
    texFile << "\\end{document}\n";
    return true;
}