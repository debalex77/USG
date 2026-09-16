UPDATE tableKidney SET
    contour_right         = :contour_right,
    contour_left          = :contour_left,
    dimens_right          = :dimens_right,
    dimens_left           = :dimens_left,
    corticomed_right      = :corticomed_right,
    corticomed_left       = :corticomed_left,
    pielocaliceal_right   = :pielocaliceal_right,
    pielocaliceal_left    = :pielocaliceal_left,
    formations            = :formations,
    suprarenal_formations = :suprarenal_formations,
    concluzion            = :concluzion,
    recommendation        = :recommendation
WHERE
    id_reportEcho = :id_reportEcho