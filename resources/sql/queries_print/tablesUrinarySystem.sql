SELECT
    rp.docPresentationDate AS title_report,
    /* ----------------- kidney ----------------- */
    tk.contour_right,
    tk.contour_left,
    tk.dimens_right,
    tk.dimens_left,
    tk.corticomed_right,
    tk.corticomed_left,
    tk.pielocaliceal_right,
    tk.pielocaliceal_left,
    tk.formations AS kidney_formations,
    tk.suprarenal_formations,
    /* ----------------- bladder ----------------- */
    tb.volum      AS bladder_volum,
    tb.walls      AS bladder_walls,
    tb.formations AS bladder_formations,
    /* ----------------- other ------------------- */
    tk.concluzion     AS urinary_system_concluzion,
    tk.recommendation AS recommendation,
    r.concluzion
FROM
    reportEcho r
INNER JOIN
    reportEchoPresentation rp ON rp.id_reportEcho = r.id
LEFT JOIN
    tableKidney tk ON tk.id_reportEcho = r.id
LEFT JOIN
    tableBladder tb ON tb.id_reportEcho = r.id
WHERE
    r.deletionMark = 2 AND
    r.id = %id%
