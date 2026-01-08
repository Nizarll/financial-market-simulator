#pragma once

#include "widget.hpp"
#include "utils.hpp"

struct HeaderProps {
  std::function<void()> toggle_theme;
};

static auto Header(HeaderProps props) {
  auto theme_toggle_clicked = [props](bool){
    props.toggle_theme();
  };

  return HLayout {
    Text{ "Stock Price Simulation" },
    Button { "Toggle Theme" }.clicked(std::move(theme_toggle_clicked)),
  }.justifyBetween();
}
