UPDATE tableGestation2_cranium SET
    calloteCranium     = :calloteCranium,
    facialeProfile     = :facialeProfile,
    nasalBones         = :nasalBones,
    nasalBones_dimens  = :nasalBones_dimens,
    eyeball            = :eyeball,
    eyeball_desciption = :eyeball_desciption,
    nasolabialTriangle = :nasolabialTriangle,
    nasolabialTriangle_description = :nasolabialTriangle_description,
    nasalFold = :nasalFold
WHERE
    id_reportEcho = :id_reportEcho