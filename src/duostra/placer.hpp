/****************************************************************************
  FileName     [ placer.hpp ]
  PackageName  [ duostra ]
  Synopsis     [ Define class Placer structure ]
  Author       [ Chin-Yi Cheng, Chien-Yi Yang, Ren-Chu Wang, Yi-Hsiang Kuo ]
  Paper        [ https://arxiv.org/abs/2210.01306 ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <memory>
#include <random>
#include <vector>

class Device;

class BasePlacer {
public:
    BasePlacer() {}
    BasePlacer(const BasePlacer&) = delete;
    BasePlacer(BasePlacer&&) = delete;
    virtual ~BasePlacer() {}

    std::vector<size_t> placeAndAssign(Device&);

protected:
    virtual std::vector<size_t> place(Device&) const = 0;
};

class RandomPlacer : public BasePlacer {
public:
    ~RandomPlacer() override {}

protected:
    std::vector<size_t> place(Device&) const override;
};

class StaticPlacer : public BasePlacer {
public:
    ~StaticPlacer() override {}

protected:
    std::vector<size_t> place(Device&) const override;
};

class DFSPlacer : public BasePlacer {
public:
    ~DFSPlacer() override {}

protected:
    std::vector<size_t> place(Device&) const override;

private:
    void DFSDevice(size_t, Device&, std::vector<size_t>&, std::vector<bool>&) const;
};

std::unique_ptr<BasePlacer> getPlacer();